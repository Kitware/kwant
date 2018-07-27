/*ckwg +5
 * Copyright 2011-2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

/*
In order to generate an event ROC on two sets, G and C, of tracks, the following conditions
must be met:

1) The two sets must be scorable by phase 1, i.e. have boxes and
commensurate timestamps.

2) The two sets must either have been externally filtered to contain only
the activity of interest, or must contain a track-level <int>("activity") containing
an activity index which is used to filter the set.

3) Computed set C must also also contain some track field<double> to sort and use for
sweeping an ROC.  For KSTs, this is "relevancy" on the assumption that all tracks in
C were selected to be the ground-truth activity by e.g. IQR; for kwxmls, the probability of
the activity index is used.

 */

#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <limits>

#include <vul/vul_arg.h>
#include <vul/vul_file.h>
#include <vul/vul_sprintf.h>
#include <vul/vul_awk.h>
#include <vul/vul_reg_exp.h>
#include <vul/vul_timer.h>

#include <vgl/vgl_area.h>

#include <arrows/kpf/yaml/kpf_reader.h>
#include <arrows/kpf/yaml/kpf_yaml_parser.h>
#include <arrows/kpf/yaml/kpf_packet_header.h>
#include <arrows/kpf/yaml/kpf_parse_utils.h>

#include <kwiversys/RegularExpression.hxx>

#include <track_oracle/core/track_oracle_core.h>
#include <track_oracle/core/track_base.h>
#include <track_oracle/core/track_field.h>
#include <track_oracle/file_formats/kpf_utils/kpf_utils.h>
#include <track_oracle/file_formats/track_kst/track_kst.h>
#include <track_oracle/file_formats/track_xgtf/track_xgtf.h>
#include <track_oracle/file_formats/track_kwxml/track_kwxml.h>
#include <track_oracle/file_formats/track_kw18/track_kw18.h>
#if KWANT_ENABLE_KWE
#include <track_oracle/file_formats/track_filter_kwe/track_filter_kwe.h>
#endif
#include <track_oracle/file_formats/file_format_schema.h>
#include <track_oracle/file_formats/file_format_manager.h>
#include <track_oracle/utils/logging_map.h>
#include <track_oracle/data_terms/data_terms.h>
#include <track_oracle/aries_interface/aries_interface.h>
#include <track_oracle/file_formats/track_filter_kpf_activity/track_filter_kpf_activity.h>

#include <scoring_framework/score_phase1.h>
#include <scoring_framework/score_tracks_hadwav.h>
#include <scoring_framework/score_tracks_loader.h>
#include <scoring_framework/matching_args_type.h>
#include <scoring_framework/timestamp_utilities.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <vital/config/config_block.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::back_inserter;
using std::cout;
using std::endl;
using std::fstream;
using std::getline;
using std::ifstream;
using std::istringstream;
using std::map;
using std::max;
using std::numeric_limits;
using std::ofstream;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::make_pair;
using std::replace;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::transform;
using std::vector;

using kwiver::track_oracle::aries_interface;
using kwiver::track_oracle::frame_handle_list_type;
using kwiver::track_oracle::field_handle_type;
using kwiver::track_oracle::oracle_entry_handle_type;
using kwiver::track_oracle::track_field;
using kwiver::track_oracle::track_handle_list_type;
using kwiver::track_oracle::track_handle_type;
using kwiver::track_oracle::file_format_schema_type;
using kwiver::track_oracle::file_format_enum;
using kwiver::track_oracle::file_format_manager;
using kwiver::track_oracle::track_oracle_core;
using kwiver::track_oracle::track_kwxml_type;
using kwiver::track_oracle::track_kst_type;
using kwiver::track_oracle::aries_interface_exception;
using kwiver::track_oracle::track_filter_kpf_activity;

using kwiver::vital::kpf::packet_header_t;

namespace { // anon

using namespace kwiver::kwant;

enum class activity_style { INVALID, VIRAT, VIRAT_AND_EXIT, PVO, PREFILTERED, KPF_ACTIVITY, KPF_OBJECT, KPF_ADHOC };

struct pvo_request_type
{
  bool valid;
  string pvo_filename;
  size_t pvo_index;
  pvo_request_type():
    valid( false ), pvo_filename(""), pvo_index(0)
  {}
  bool set( const string& arg );
  track_handle_list_type process( const track_handle_list_type& tracks, bool is_gt ) const;
};

struct activity_selector_type
{
  activity_style style;
  int activity_index;    // for VIRAT style
  string activity_name;  // for KPF style
  string kpf_adhoc_field; // for KPF adhoc
  int activity_domain;   // for KPF style
  packet_header_t kpf_conf_src; // for KPF style
  pvo_request_type pvo_req; // for PVO style

  activity_selector_type()
    : style( activity_style::INVALID ), activity_index(0), activity_name("invalid")
  {}
};

//
// Abstract base class for various sources of relevancy
//

class relevancy_extractor_type
{
public:
  relevancy_extractor_type() {}
  virtual bool valid() const { return true; }
  virtual ~relevancy_extractor_type() {}
  virtual pair<bool, double> get( oracle_entry_handle_type row ) = 0;
};

//
// returns a fixed value (typically 1.0 for ground truth)
//

class fixed_extractor_type: public relevancy_extractor_type
{
public:
  explicit fixed_extractor_type( double v ): val(v) {}
  virtual pair<bool, double> get( oracle_entry_handle_type ) { return make_pair( true, this->val ); }
private:
  double val;
};

//
// obtains the value from a single column (which stores a double)
//

class simple_field_extractor_type: public relevancy_extractor_type
{
public:
  explicit simple_field_extractor_type( field_handle_type fh): f(fh) {}
  virtual pair<bool, double> get( oracle_entry_handle_type row )
  {
    return track_oracle_core::get<double>( row, this->f );
  }
private:
  field_handle_type f;
};

//
// obtains the value via a KPF cset
//
// KPF csets implemented (per track_oracle kpf_utils.cxx) as TWO fields:
// each row has a map of size_t -> double, and a single instance
// of a map resolving user-side strings to the size_t key of the per-row field.
//

class kpf_cset_extractor_type: public relevancy_extractor_type
{
public:
  kpf_cset_extractor_type( const string& kpf_header_string, const string& target, bool zero_if_absent );
  virtual bool valid() const { return this->valid_flag; }
  virtual pair<bool, double> get( oracle_entry_handle_type row );
private:
  field_handle_type f; // field for the size_t -> double map
  pair< bool, size_t> target_index; // index of the target string in the map@f (if true)
  bool zero_flag;      // if true, return <true, 0.0> if the string isn't there
  bool valid_flag;
};


//
// obtains the value via a KPF activity
//
// KPF activities are stored directly as a map of <string, double>
//
class kpf_activity_extractor_type: public relevancy_extractor_type
{
public:
  kpf_activity_extractor_type( field_handle_type fh, const string& t, bool zero_if_absent );
  virtual pair< bool, double> get( oracle_entry_handle_type row );
private:
  field_handle_type f; // field for the actual string -> double map
  string target;       // string we're looking for
  bool zero_flag;      // if true, return <true, 0.0> if string isn't there
};

//
// obtains the value via a track_field directly holding a cset
//
class kpf_adhoc_extractor_type: public relevancy_extractor_type
{
public:
  kpf_adhoc_extractor_type( const string& field_name, const string& t, bool zero_if_absent );
  virtual bool valid() const { return this->valid_flag; }
  virtual pair< bool, double > get( oracle_entry_handle_type row );
private:
  field_handle_type f; // field for the field_name
  string target;       // string we're looking for
  bool zero_flag;      // if true, return <true, 0.0> if the string isn't there
  bool valid_flag;
};

//
// kpf_adhoc implementation
//

kpf_adhoc_extractor_type
::kpf_adhoc_extractor_type( const string& field_name,
                            const string& t,
                            bool zero_if_absent )
  : target( t ), zero_flag( zero_if_absent ), valid_flag( false )
{
  // lookup the field for the cset; return if not found
  this->f = track_oracle_core::lookup_by_name( field_name );
  if ( this->f == kwiver::track_oracle::INVALID_FIELD_HANDLE) return;

  this->valid_flag = true;
}

pair< bool, double >
kpf_adhoc_extractor_type
::get( oracle_entry_handle_type row )
{
  if ( ! this->valid_flag ) return make_pair( false, 0.0 );

  const auto& m = track_oracle_core::get< kwiver::track_oracle::kpf_cset_type >( row, this->f );
  if ( ! m.first ) return make_pair( false, 0.0 );

  const auto& p = m.second.find( this->target );
  return
    p != m.second.end()
    ? make_pair( true, p->second )
    : make_pair( this->zero_flag, 0.0 );
}

//
// kpf_cset implementation
//

kpf_cset_extractor_type
::kpf_cset_extractor_type( const string& kpf_header_string,
                           const string& target,
                           bool zero_if_absent)
  : zero_flag( zero_if_absent ), valid_flag( false )
{
  // lookup the field for per-row index->double table, return if not found
  this->f = track_oracle_core::lookup_by_name( kpf_header_string );
  if (this->f == kwiver::track_oracle::INVALID_FIELD_HANDLE) return;

  // lookup the single instance of string->index, return if not found
  auto s2i_fh = track_oracle_core::lookup_by_name( kpf_header_string+"_s2i" );
  if (s2i_fh == kwiver::track_oracle::INVALID_FIELD_HANDLE) return;

  // obtain the string->index map; return if not found
  const auto& s2i_map =
    track_oracle_core::get< map<string, size_t > >(
      kwiver::track_oracle::SYSTEM_ROW_HANDLE,
      s2i_fh );
  if ( ! s2i_map.first ) return;

  // lookup and cache the index of the target string
  // target_index is a pair; set the first to false when we have a valid
  // string which nobody ever reported and therefore isn't in the map
  auto p = s2i_map.second.find( target );
  this->target_index =
    p == s2i_map.second.end()
    ? make_pair( false, static_cast<size_t>(0) )
    : make_pair( true, p->second );

  // all set!
  this->valid_flag = true;
}


pair< bool, double >
kpf_cset_extractor_type
::get( oracle_entry_handle_type row )
{
  if ( ! this->valid_flag ) return make_pair( false, 0.0 );

  // get the per-row size_t -> double map
  const auto& m = track_oracle_core::get< map< size_t, double > >( row, this->f );
  if ( ! m.first ) return make_pair( false, 0.0 );

  // if the target index is false (i.e. it's a string we NEVER saw...)
  if (! this->target_index.first )
  {
    // ... return (true, 0.0) if zero-if-absent flag is set, (false, 0.0) otherwise
    return make_pair( this->zero_flag, 0.0 );
  }

  // lookup the index in the map, return value if found,
  // otherwise return value conditioned on the zero-if-absent-flag
  auto p = m.second.find( this->target_index.second );
  return
    p != m.second.end()
    ? make_pair( true, p->second )
    : make_pair( this->zero_flag, 0.0 );
}

//
// kpf_activity implementation
//

kpf_activity_extractor_type
::kpf_activity_extractor_type( field_handle_type fh, const string& t, bool zero_if_absent ):
  f(fh), target(t), zero_flag( zero_if_absent )
{
}

pair< bool, double >
kpf_activity_extractor_type
::get( oracle_entry_handle_type row )
{
  // get the per-row string->double map
  const auto& m = track_oracle_core::get< kwiver::track_oracle::kpf_cset_type >( row, this->f );
  if ( ! m.first ) return make_pair( false, 0.0 );

  // look for the string; return val if found, else according to zero_flag
  auto p = m.second.find( this->target );
  return
    p == m.second.end()
    ? make_pair( true, p->second )
    : make_pair( this->zero_flag, 0.0 );
}


ostream& operator<<( ostream& os, const activity_selector_type& a )
{
  switch (a.style)
  {
  case activity_style::INVALID:
    os << "invalid";
    break;
  case activity_style::VIRAT:
    os << "VIRAT: index " << a.activity_index;
    break;
  case activity_style::VIRAT_AND_EXIT:
    os << "VIRAT (listing activities and exit)";
    break;
  case activity_style::PVO:
    os << "PVO (index " << a.pvo_req.pvo_index << ")";
    break;
  case activity_style::PREFILTERED:
    os << "prefiltered";
    break;
  case activity_style::KPF_ACTIVITY:
    os << "KPF " << a.activity_name << " / " << a.activity_domain << " via activity";
    break;
  case activity_style::KPF_OBJECT:
    os << "KPF " << a.activity_name << " / " << a.activity_domain << " via object";
    break;
  case activity_style::KPF_ADHOC:
    os << "KPF adhoc: " << a.activity_name << " via field " << a.kpf_adhoc_field;
  default:
    os << "UNKNOWN?";
    break;
  }
  return os;
}

bool
pvo_request_type
::set( const string& arg )
{
  if (arg == "" ) return true;
  size_t colon_idx = arg.find( ':' );
  if ((colon_idx == string::npos) || (colon_idx != arg.size()-2))
  {
    LOG_ERROR( main_logger, "Couldn't parse pvo request type '" << arg << "'" );
    return false;
  }
  this->pvo_filename = arg.substr( 0, colon_idx );
  string pvo_flag = arg.substr( colon_idx+1, 1 );
  string tr_src = "PpVvOo";
  size_t tr_dst[] = {0, 0, 1, 1, 2, 2};
  size_t pvo_pos = tr_src.find( pvo_flag[0] );
  if (pvo_pos == string::npos)
  {
    LOG_ERROR( main_logger, "Bad PVO flag '" << pvo_flag[0] << "'; must be one of '" << tr_src << "'" );
    return false;
  }
  else
  {
    this->pvo_index = tr_dst[ pvo_pos ];
  }
  LOG_INFO( main_logger, "Will read PVO values from '" << pvo_filename << "' using " << pvo_flag << " value" );
  this->valid = true;
  return true;
}

track_handle_list_type
pvo_request_type
::process( const track_handle_list_type& tracks,
           bool is_gt ) const
{
  track_field< double > relevancy( "relevancy" );

  if (is_gt)
  {
    for (size_t i=0; i<tracks.size(); ++i)
    {
      relevancy( tracks[i].row ) = 1.0;
    }
  }
  else
  {
    // build up our own little ad-hoc domain for the track IDs
    track_field< unsigned > external_id( "external_id" );
    map< unsigned, oracle_entry_handle_type > id_map;
    for (size_t i=0; i<tracks.size(); ++i)
    {
      oracle_entry_handle_type h = tracks[i].row;
      pair< bool, unsigned > probe = external_id.get( h );
      if ( ! probe.first )
      {
        throw runtime_error( "Cannot set PVO values on computed tracks without external IDs" );
      }
      if (id_map.find( probe.second ) != id_map.end())
      {
        ostringstream oss;
        oss << "PVO: computed track set has duplicate track ID " << probe.second;
        throw runtime_error( oss.str().c_str() );
      }

      id_map[ probe.second ] = h;
    }

    // load up the pvo map
    ifstream is( this->pvo_filename.c_str() );
    if ( ! is )
    {
      throw runtime_error( "Couldn't open PVO filename '" + this->pvo_filename + "'" );
    }
    string tmp;
    while (getline( is, tmp ))
    {
      replace( tmp.begin(), tmp.end(), ',', ' ');
      istringstream iss( tmp );
      unsigned id;
      double pvo[3];
      if (!( iss >> id >> pvo[0] >> pvo[1] >> pvo[2] ))
      {
        LOG_WARN( main_logger, "Couldn't parse PVO value from string '" << tmp << "'" );
      }
      map< unsigned, oracle_entry_handle_type >::iterator probe = id_map.find( id );
      if ( probe == id_map.end())
      {
        LOG_WARN( main_logger, "PVO map refers to non-existent track " << id << "; skipping" );
        continue;
      }

      // set the value
      relevancy( probe->second ) = pvo[ this->pvo_index ];
      // remove
      id_map.erase( probe );
    }

    if (! id_map.empty() )
    {
      LOG_WARN( main_logger, "Warning: " << id_map.size() << " input tracks were not referenced in the PVO map" );
    }
  }

  return tracks;
}

} // anon

using namespace kwiver::kwant;

struct output_args_type
{
  vul_arg< string > track_stats_fn;
  vul_arg< string > target_stats_fn;
  vul_arg< string > matches_dump_fn;
  vul_arg< string > ts_dump_fn;
  vul_arg< string > roc_dump_fn;
  vul_arg< string > roc_csv_dump_fn;
  vul_arg< string > pr_dump_fn;
  vul_arg< string > thresholds_arg;
  vul_arg< bool > console_dump_arg;


  output_args_type()
    : track_stats_fn(  "--track-stats", "write track purity / continuity to file" ),
      target_stats_fn( "--target-stats", "write target purity / continuity to file" ),
      matches_dump_fn( "--matches", "write track match info to file" ),
      ts_dump_fn( "--ts-dump", "write ground-truth frame -> timestamps to file" ),
      roc_dump_fn( "--roc-dump", "write the roc chart information to file" ),
      roc_csv_dump_fn( "--roc-csv-dump", "write the roc chart information to file (CSV format)" ),
      pr_dump_fn( "--pr-dump", "write the P/R curve information to file (if not set, dump to cout" ),
      thresholds_arg( "--thresholds", "Manually specified thresholds to score on (min[:max[:step]])" ),
      console_dump_arg( "--console", "Write ROC lines to the console" )
  {}
};

void
compute_roc( const track2track_phase1& p1,
             const track_handle_list_type& truth_tracks,
             const track_handle_list_type& computed_tracks,
             double fa_norm,
             int max_n_roc_points,
             output_args_type& output_args );

void
compute_pr( const track2track_phase1& p1,
            track_handle_list_type truth_tracks,
            track_handle_list_type computed_tracks,
            output_args_type& output_args );

//
// There are two types of filtering we need to support.
//
// Track-style filtering typically only applies to kwxml files;
// it allows tracks of (say) "trackKWE" to be selected from a
// set of larger files.
//
// Activity filtering is used to select tracks which contain a given
// activity.  Currently, the downstream scoring code will be looking
// for a single value (1.0 for ground truth, [0..1] for computed
// activity measures) in the "relevancy" slot introduced by the KST
// format.  There are three paths to populating the relevancy slot:
//
// 1) It's already there (i.e. a KST file.)  Do nothing.
//
// 2) If the track contains both activity() and activity_probability()
// fields (i.e. xgtf), discard the track if activity() doesn't match
// the --a parameter.  If it matches, keep the track and copy the
// probability over to the relevancy slot.
//
// 3) If the track contains a descriptor_classifier, keep the track
// and copy the activity probability over to relevancy.
//
// If both (2) and (3) are true, it's an error.
//
// If none of (1), (2), or (3) is true, it's an error.
//
// If track_style is present in the track but no track-style filtering
// is selected, that's an error.  (Allow ':all:' to bypass the filter
// and, therefore, accept all track styles.)
//


//
// utility routine to return the source of a track
//
string
get_track_source( const track_handle_type& h )
{
  file_format_schema_type fst;
  string source_file = "undefined";
  if ( fst.source_file_id.exists( h.row ))
  {
    source_file = file_format_schema_type::source_id_to_filename(
      fst( h ).source_file_id() );
  }
  return source_file;
}


//
// Filter on track_style.  If the track doesn't define track_style,
// copy.  If it defines it but no argument is specified, throw.
// Otherwise, copy if the style matches or the argument is ':all:'.
//

track_handle_list_type
filter_on_track_style( const track_handle_list_type& input_tracks,
                       vul_arg<string>& track_style_arg )
{
  track_field< kwiver::track_oracle::dt::tracking::track_style > track_style;
  const string any_style = ":all:";
  track_handle_list_type ret;

  for (size_t i=0; i<input_tracks.size(); ++i)
  {
    const oracle_entry_handle_type& row = input_tracks[i].row;

    // if the track does not define track style, copy it and continue.
    if ( ! track_style.exists( row ))
    {
      ret.push_back( input_tracks[i] );
      continue;
    }

    // otherwise, if the track style argument isn't set, throw an error.
    if ( ! track_style_arg.set() )
    {
      string source_file = get_track_source( input_tracks[i] );
      throw runtime_error( "At least one track in source '" + source_file + "' defines" +
                               " track_style but the '" + track_style_arg.option() +
                               "' option is not set.\nPlease select a track style or use '" +
                               any_style +"' to select all tracks regardless of style.\n" );
    }

    // If we get here, the track defines a style and the user has selected
    // a style.  If they match, copy it over.
    string selected_style = track_style_arg();
    if ( (selected_style == any_style) || (track_style(row) == selected_style))
    {
      ret.push_back( input_tracks[i] );
    }
  }

  return ret;
}

//
// Filter the input tracks such that every output track:
// (a) has a probability / likelihood / whatever defined for the
//     user-selected activity
// (b) that probability is copied into the relevance() slot.
//
// For ground truth tracks, we only want to keep tracks whose
// value in (a) is 1.0.  For computed tracks, we'll take them
// all (assuming appropriate descriptor-level filtering was handled
// by filter_on_track_style.)
//
// The p2r flag is only used if the file is KST.  Previously,
// it was used to convert kwxml descriptor probs to KST relevancy,
// but now that always happens implicitly.  p2r is only when a KST
// file contains relevancy unrelated to the descriptor classifier,
// i.e. some sort of IQR return ranking.
//
// pvo requests are special-cased by loading the pvo file and plugging
// 1.0 as relevancy for ground-truth or the appropriate p/v/o value as
// relevancy for computed.
//
// If the input is prefiltered, we assume it has relevancy externally
// set as well. (If we're ground-truth, we'll set it to 1.0.)
//

track_handle_list_type
normalize_activity_tracks( const track_handle_list_type& input_tracks,
                           bool input_is_gt,
                           bool input_is_prefiltered,
                           bool probability_to_relevancy,
                           const activity_selector_type& what_act )
{
  //
  // KPF activity tracks have already been normalized
  //

  if (what_act.style == activity_style::KPF_ACTIVITY)
  {
    return input_tracks;
  }

  if ( what_act.pvo_req.valid )
  {
    return what_act.pvo_req.process( input_tracks, input_is_gt );
  }

  kwiver::logging_map_type wmap( main_logger, KWIVER_LOGGER_SITE );
  wmap.set_output_prefix( "normalize_activity_tracks: " );

  // (1) from xgtf
  track_field< kwiver::track_oracle::dt::events::event_type > activity;
  track_field< kwiver::track_oracle::dt::events::event_probability > activity_probability;

  // (2) from kwxml
  track_field< vector<double> > descriptor_classifier( "descriptor_classifier" );

  // (3) from kst
  track_field< double > relevancy( "relevancy" );
  enum {NONE = 0, HAS_XGTF = 1, HAS_KWXML = 2, HAS_KST = 4};

  track_handle_list_type ret;

  //
  // KPF objects
  //

  if (what_act.style == activity_style::KPF_OBJECT)
  {
    ostringstream oss;
    oss << kwiver::vital::kpf::style2str( kwiver::vital::kpf::packet_style::CSET )
        << "_"
        << what_act.activity_domain;

    // for ground-truth, absence means exclude
    // for computed, absence means assume 0.0
    bool zero_if_absent = (! input_is_gt );

    relevancy_extractor_type* r =
      new kpf_cset_extractor_type( oss.str(), what_act.activity_name, zero_if_absent );
    LOG_DEBUG( main_logger, "KPF cset extractor from " << oss.str() << " for " << what_act );
    for (const auto& i: input_tracks )
    {
      size_t match_count = 0;
      double last_r = 0.0;
      for (const auto &f: track_oracle_core::get_frames( i ))
      {
        auto p = r->get( f.row );
        if (input_is_prefiltered || p.first)
        {
          relevancy( f.row ) = p.second;
          last_r = p.second;
          ++match_count;
        }
      }
      if (match_count == track_oracle_core::get_n_frames( i ))
      {
        //        LOG_DEBUG( main_logger, "all frames match; setting track to " << last_r );
        relevancy( i.row ) = last_r;
        ret.push_back( i );
      }
      else if (match_count > 0)
      {
        LOG_WARN( main_logger, "Inconsistent detection flags for multi-detection track: matched "
                  << match_count << " detections out of " << track_oracle_core::get_n_frames( i )
                  << "; not counting as match" );
      }
      else if (match_count == 0)
      {
        // do nothing, it's just a straight miss
      }
    }
    delete r;
    return ret;
  }

  //
  // KPF adhoc
  //

  if (what_act.style == activity_style::KPF_ADHOC)
  {
    // for ground-truth, absence means exclude
    // for computed, absence means assume 0.0
    bool zero_if_absent = (! input_is_gt );

    relevancy_extractor_type* r =
      new kpf_adhoc_extractor_type( what_act.kpf_adhoc_field, what_act.activity_name, zero_if_absent );

    LOG_INFO( main_logger, "KPF adhoc extractor for " << what_act.activity_name << " from " << what_act.kpf_adhoc_field << ": valid? " << r->valid() );
    if (! r->valid())
    {
      delete r;
      return ret;
    }

    for (const auto& i: input_tracks )
    {
      size_t match_count = 0;
      double last_r = 0.0;
      for (const auto& f: track_oracle_core::get_frames( i ))
      {
        auto p = r->get( f.row );
        if (input_is_prefiltered || p.first )
        {
          relevancy( f.row ) = p.second;
          last_r = p.second;
          ++match_count;
        }
      }
      if (match_count == track_oracle_core::get_n_frames( i ))
      {
        //        LOG_DEBUG( main_logger, "all frames match; setting track to " << last_r );
        relevancy( i.row ) = last_r;
        ret.push_back( i );
      }
      else if (match_count > 0)
      {
        LOG_WARN( main_logger, "Inconsistent detection flags for multi-detection track: matched "
                  << match_count << " detections out of " << track_oracle_core::get_n_frames( i )
                  << "; not counting as match" );
      }
      else if (match_count == 0)
      {
        // do nothing; it's just a straight miss
      }
    }

    delete r;
    return ret;
  }

  for (size_t i=0; i<input_tracks.size(); ++i)
  {
    const oracle_entry_handle_type& row = input_tracks[i].row;

    // quick exit if prefiltered set and we have relevancy (
    if (input_is_prefiltered)
    {
      ret.push_back( input_tracks[i] );
      if (input_is_gt)
      {
        relevancy( row ) = 1.0;
      }
      else
      {
        if (! relevancy.exists(row ))
        {
          wmap.add_msg( "Prefiltered computed track has no relevancy" );
        }
      }
      continue;
    }

    // Must have one of (1) or (2) or (3)
    int flag = NONE;
    if (activity.exists( row ) && activity_probability.exists( row )) flag += HAS_XGTF;
    if ((! relevancy.exists( row ) ) && descriptor_classifier.exists( row )) flag += HAS_KWXML;
    if (relevancy.exists( row ) && descriptor_classifier.exists( row )) flag += HAS_KST;

    bool keep_this_track = false;

    switch (flag)
    {

    case HAS_XGTF:
      {
        if (what_act.style == activity_style::VIRAT)
        {
          wmap.add_msg( "track activity style XGTF" );
          if (activity( row ) == what_act.activity_index )
          {
            double p = activity_probability( row );
            if ((input_is_gt) && (p != 1.0))
            {
              LOG_WARN( main_logger, "XGTF-style probability for activity " << what_act.activity_index << " is " << p << "?" );
            }
            relevancy( row ) = p;
            keep_this_track = true;
          }
        }
        else
        {
          ostringstream oss;
          oss << "XGTF input detected but activity selector is " << what_act;
          wmap.add_msg( oss.str() );
        }
      }
      break;

    case HAS_KWXML:
      {
        if (what_act.style == activity_style::VIRAT)
        {
          wmap.add_msg( "track activity style kwxml" );
          vector< double > probs = descriptor_classifier( row );
          double p = probs [ what_act.activity_index ];
          if (input_is_gt)
          {
            // If we're claiming this is ground truth, only keep tracks of the
            // desired style whose probability is 1.0
            keep_this_track = ( p == 1.0 );
          }
          else
          {
            // If it's not ground truth, always keep
            keep_this_track = true;
          }
          if (keep_this_track)
          {
            relevancy( row ) = p;
          }
        }
        else
        {
          ostringstream oss;
          oss << "KWXML input detected but activity selector is " << what_act;
          wmap.add_msg( oss.str() );
        }
      }
      break;

    case HAS_KST:
      {
        if (what_act.style == activity_style::VIRAT)
        {
          wmap.add_msg( "track activity style kst" );
          if (probability_to_relevancy)
          {
            vector< double > probs = descriptor_classifier( row );
            double p = probs [ what_act.activity_index ];
            if ((input_is_gt) && (p != 1.0))
            {
              LOG_WARN( main_logger, "KWXML-style probability for activity " << what_act.activity_index << " is " << p << "?" );
            }
            relevancy( row ) = p;
          }
          keep_this_track = true;
        }
        else
        {
          ostringstream oss;
          oss <<  "KST input detected but activity selector is " << what_act;
          wmap.add_msg( oss.str() );
        }
      }
      break;

    case NONE:
      {
        wmap.add_msg( "track activity style none" );
        string source_file = get_track_source( input_tracks[i] );
        file_format_schema_type fst;
        bool add_empty_classifier = false;
        if (fst.format.exists( input_tracks[i].row ))
        {
          add_empty_classifier = (fst.format( input_tracks[i].row ) == kwiver::track_oracle::TF_KWXML);
        }

        if (add_empty_classifier)
        {
          vector< double > activity_probabilities;
          activity_probabilities.resize( aries_interface::index_to_activity_map().size(), 0.0 );
          descriptor_classifier( row ) = activity_probabilities;
          ostringstream oss;
          oss << "Added empty classifier to track from " << source_file;
          wmap.add_msg( oss.str() );
        }
        else
        {
          // I don't think this is a problem?
          /*
          throw runtime_error( "At least one track in '" + source_file + "' does not define " +
                                   "either xgtf or kwxml/kst activities; can't be used in score_events.");
          */
        }

      }
      break;

    default:
      {
        string source_file = get_track_source( input_tracks[i] );
        throw runtime_error( "At least one track in '" + source_file + "' defines " +
                                 "both xgtf-acitivty and kwxml/kst descriptor classifier fields;\n" +
                                 "no workaround defined yet.");
      }
    } // switch

    if (keep_this_track)
    {
      ret.push_back( input_tracks[i] );
    }

  } // ...for all tracks

  wmap.dump_msgs();

  return ret;
}


// Link (kwxml) tracks "K" via their source track ID.
// Assume all K tracks are the same track style; the intent is to
// create a single e.g. PVMoving track from all the PVMoving segments
// with the same source track ID, while ignoring the ICSIhog tracks and
// SOTI2 tracks and so forth.
//
// Assume K tracks have a single source track ID S.  If number of
// source tracks != 1, emit warnings.  Otherwise, create new kwxml
// track with id S, frames from K tracks, taking (for the moment) a
// descriptor classifier whose each slot is the maxmimum of that slot
// across all K tracks.  This isn't normalized or anything, but it's
// close enough for now.  (I think.)
//
// The other interesting catch is that the pool of k_tracks may arise
// from multiple descriptor files, each with their own domain of
// track IDs, leading to ambiguity.  The file_format API records the
// filename of each track as an ID, so we should be able to use those
// to disambiguate.
//
// Since this is called after any activities have been promoted to relevancies,
// we also have to optionally copy over the maximium activity index probability
// as the relevancy field.
//

track_handle_list_type
relink_kwxml_on_source_ids( const track_handle_list_type& k_tracks, int activity_index_to_relevancy )
{
  track_handle_list_type ret;

  typedef pair< unsigned, unsigned > k_track_key_type;
  // k_track_key_type.first == source file ID
  // k_track_key_type.second == KWXML's source track ID

  // map <track_source_file_id, source_track_IDs> to constituent k tracks
  map< k_track_key_type, track_handle_list_type > src2k_map;
  typedef map< k_track_key_type, track_handle_list_type >::const_iterator src2k_map_cit;

  track_field< kwiver::track_oracle::dt::events::source_track_ids > src_track_ids_field;
  file_format_schema_type ts;

  unsigned max_external_id = 0;

  for (unsigned i=0; i<k_tracks.size(); ++i)
  {
    const track_handle_type t = k_tracks[i];
    // build the key
    if (! ts.source_file_id.exists( t.row ))
    {
      LOG_ERROR( main_logger, "Logic error: no source file ID for a track?");
      return track_handle_list_type();
    }
    if (! src_track_ids_field.exists( t.row ))
    {
      LOG_INFO( main_logger, "Ordinal k_track " << i << ": No source track id?");
      continue;
    }

    const vector< unsigned >& src_track_ids = src_track_ids_field( t.row );
    size_t n = src_track_ids.size();
    if ( n != 1 )
    {
      LOG_INFO( main_logger, "Ordinal k_track " << i << ": contains " << n << " source track IDs?");
      continue;
    }

    k_track_key_type key( ts.source_file_id( t.row ), src_track_ids[0] );

    // all we need the key for is to keep the lists of k_tracks separate
    // we generate new external IDs since I'm not 100% certain the descriptor
    // track ID domain is separate from the vidtk tracker's track ID domain.
    // they should be separate, but might not be, so we'll just make up
    // new ones for now
    if (key.second > max_external_id) max_external_id = key.second;
    src2k_map[ key ].push_back( t );
  }

  // loop over each source track ID, add frames, and set the classifier
  track_kwxml_type kwxml;
  for (src2k_map_cit i = src2k_map.begin(); i != src2k_map.end(); ++i)
  {
    track_handle_type new_track = kwxml.create();
    vector<double> new_classifier;

    const track_handle_list_type& src_k_tracks = i->second;
    for (unsigned j=0; j<src_k_tracks.size(); ++j)
    {
      track_handle_type this_k_track = src_k_tracks[j];
      kwxml( new_track ).add_frames( track_oracle_core::get_frames( this_k_track ));

      if ( ! kwxml( this_k_track ).descriptor_classifier.exists() )
      {
        LOG_ERROR( main_logger, "Logic error: no classifier in:\n" << kwxml(this_k_track) << "");
        return track_handle_list_type();
      }

      const vector<double>& this_classifier = kwxml( this_k_track ).descriptor_classifier();
      if ( j == 0 )
      {
        new_classifier = this_classifier;
      }
      else
      {
        if (new_classifier.size() != this_classifier.size() )
        {
          LOG_ERROR( main_logger, "Logic error: intra-segment classifier size mismatch; " << new_classifier.size()
                   << " vs " << this_classifier.size() << "");
        }
        else
        {
          for (unsigned k=0; k<this_classifier.size(); ++k)
          {
            new_classifier[k] = max( this_classifier[k], new_classifier[k] );
          }
        }
      }
    } // ... for each of the k tracks

    kwxml(new_track).external_id() = ++max_external_id;
    kwxml(new_track).descriptor_classifier() = new_classifier;
    if ( activity_index_to_relevancy != -1 )
    {
      track_kst_type kst_schema;
      kst_schema( new_track ).relevancy() = new_classifier[ activity_index_to_relevancy ];
    }
    ret.push_back( new_track );

    LOG_INFO( main_logger, "source track " << i->first.first << ":" << i->first.second << " contained " << src_k_tracks.size()
             << " component tracks; now has " << track_oracle_core::get_n_frames( new_track ) << " frames");

  }

  // all done
  return ret;
}



 // Comparator function for kst track handles
// - Place kst tracks with a lower rank before those with higher rank
//   (rank is the ordering by Viqui of tracks received from IQR)
// - If rank is equal, place tracks with a higher relevancy before those
//   with less relevancy.
// - If relevancy is equal, we sort by time-stamp of first frame
//   (earlier tracks are before later tracks)
bool compare_kst_track_handle(track_handle_type i, track_handle_type j)
{
  track_kst_type kst;
  unsigned i_rank = kst( i ).rank(),
           j_rank = kst( j ).rank();

  if (i_rank != j_rank)
    return (i_rank < j_rank);

  else // Secondary sort on relevancy.
       // This should never happen as rank should be unique per track
  {
    double i_relev = kst( i ).relevancy(),
           j_relev = kst( j ).relevancy();

    if (i_relev != j_relev)
      return (i_relev > j_relev);

    else // Tertiary sort on first-frame time-stamp
    {
      frame_handle_list_type iframes = track_oracle_core::get_frames( i );
      frame_handle_list_type jframes = track_oracle_core::get_frames( j );
      double i_ts = static_cast<double>(kst[ iframes[0] ].timestamp_usecs());
      double j_ts = static_cast<double>(kst[ jframes[0] ].timestamp_usecs());

      return (i_ts < j_ts);
    }
  }
}


#ifdef KWANT_ENABLE_MGRS
void
add_tracks_to_box( const track_handle_list_type& tracks,
                   vgl_box_2d<double>& box )
{
  track_field< scorable_mgrs > mgrs( "mgrs_pos" );
  for (size_t i=0; i<tracks.size(); ++i)
  {
    frame_handle_list_type frames = track_oracle_core::get_frames( tracks[i] );
    for (size_t j=0; j<frames.size(); ++j)
    {
      pair< bool, scorable_mgrs > probe = mgrs.get( frames[j].row );
      if ( ! probe.first )
      {
        LOG_WARN( main_logger, "Normalization: truth track " << j << " has no mgrs?" );
        continue;
      }
      const scorable_mgrs& s = probe.second;
      int valid = -1;
      for (size_t k=0; k<2; ++k)
      {
        if (s.entry_valid[k])
        {
          valid = k;
          break;
        }
      }
      if (valid == -1)
      {
        LOG_WARN( main_logger, "Normalization: mgrs for " << j << " has no valid entry?" );
        continue;
      }

      box.add( vgl_point_2d<double>( s.northing[valid], s.easting[valid] ));
    }
  }
}

double
compute_normalization_factors(  const track_handle_list_type& truth_tracks,
                                const track_handle_list_type& computed_tracks )
{
  // Assume we have mgrs and all filtering has taken place.  Compute the
  // bounding box of the union of the tracks and its time window.

  vgl_box_2d<double> ne_box;  // northing / easting box; assume the zones all match
  add_tracks_to_box( truth_tracks, ne_box );
  add_tracks_to_box( computed_tracks, ne_box );
  LOG_INFO( main_logger, "Normalization: MGRS all-tracks AOI is " << vgl_area( ne_box ) << " m^2" );

  timestamp_utilities::track_timestamp_stats_type tst_t( truth_tracks ), tst_c( computed_tracks );
  tst_t.combine_with_other( tst_c );
  double time_window_secs = (tst_t.minmax_ts.second - tst_t.minmax_ts.first) / 1.0e6;
  LOG_INFO( main_logger, "Normalization: all-tracks time window is " << time_window_secs << " seconds" );

  double kmsq_in_box = vgl_area( ne_box ) / 1.0e6;

  // e.g. if area == 3 and time_window_secs = 120, then to get to FA / (km^2 per s),
  // divide FA count by (3 * 120)
  double kmsq_sec = kmsq_in_box * time_window_secs;
  LOG_INFO( main_logger, "Normalization: divide by " << kmsq_sec << " for km^2 / sec" );
  return kmsq_sec;
}
#endif

track_handle_list_type
process_top_n_tracks( track_handle_list_type computed_tracks,
                      unsigned top_n,
                      bool ct_in_rank )
{
  //
  // If top_n_arg is set and greater than 0, we want to keep just that many computed
  // tracks (sorted by "relevancy"), throw the rest away, and procceed.
  //
  if (top_n > computed_tracks.size())
  {
    LOG_INFO( main_logger, "The --top-n option was set to " << top_n
              << " but only " << computed_tracks.size() << " computed tracks were loaded;"
              << " results computed on all computed tracks");
  }
  else
  {
    if (! ct_in_rank )
    {
      // comparator (defined above) places tracks with higher relevency before those with lower.
      // secondary sort on timestamp of the first frame of the track
        sort( computed_tracks.begin(), computed_tracks.end(), compare_kst_track_handle);
    }
    else
    {
      LOG_INFO( main_logger, "assuming that the computed track file was writted pre-sorted by rank..." );
    }
    computed_tracks.erase(computed_tracks.begin() + top_n,
                          computed_tracks.end());
  }

  return computed_tracks;
}

void
process_p1_debug_dump( const track2track_phase1& p1,
                       const track_handle_list_type& truth_tracks,
                       const track_handle_list_type& computed_tracks,
                       const string& t2t_dump_fn,
                       bool disable_t2t_dump_cmd_file )
{
  LOG_INFO( main_logger, "min t2t ts: " << p1.min_ts() << "");

  p1.debug_dump( truth_tracks,
                 computed_tracks,
                 t2t_dump_fn,
                 p1.min_ts() );
  if ( ! disable_t2t_dump_cmd_file )
  {
    string cmd_fn = "plot-"+t2t_dump_fn+"-cmd.txt";
    ofstream os( cmd_fn.c_str() );
    if ( ! os )
    {
      LOG_ERROR( main_logger, "Couldn't write '" << cmd_fn << "' (not fatal, but weird)");
    }
    else
    {
      os << "plot "
         << "\"" << t2t_dump_fn << "-gt.dat\" ls 2 lw 2 w points t \"gt\", "
         << "\"" << t2t_dump_fn << "-m0.dat\" ls 1 w lines t \"matched\", "
         << "\"" << t2t_dump_fn << "-m1.dat\" w lines t \"unmatched\"\n";
      LOG_INFO( main_logger, "Info: wrote '" << cmd_fn << "'");
    }
  }
}

ostream&
operator<<( ostream& os, const track2track_type& k )
{
  track_field<unsigned>id ("external_id");
  pair<bool, unsigned> gt = id.get( k.first.row );
  pair<bool, unsigned> ct = id.get( k.second.row );
  os << "[G ";
  if (gt.first) os << gt.second; else os << "NONE";
  os << " C ";
  if (ct.first) os << ct.second; else os << "NONE";
  os << " ]";
  return os;
}

void
process_single_match_stat( ostream& os,
                           track_handle_type h,
                           bool is_gt,
                           const track2track_phase1& activity_p1,
                           const track2track_phase1& a2t_p1 )
{
  typedef map< track2track_type, track2track_score >::const_iterator p1_probe_type;

  const string axis_tag = (is_gt) ? "GA" : "CA";
  const string act_tag = (is_gt) ? "ca" : "ga";
  const string track_tag = (is_gt) ? "ct" : "gt";

  track_field< unsigned > id( "external_id" );
  track_field< kwiver::track_oracle::dt::events::source_track_ids > source_track_ids;
  if (! id.exists( h.row ))
  {
    LOG_ERROR( main_logger, "No ID for a handle? is_gt: " << is_gt );
    return;
  }

  // output preamble

  os << axis_tag << " " << id( h.row ) << " : ";

  // construct a correctly-oriented probe for the t2t_phase1 objects

  track2track_type probe( (is_gt) ? h : track_handle_type(),
                          (is_gt) ? track_handle_type() : h );

  // do we have any activity-to-activity matches?

  vector< track2track_type > a2a_keys = activity_p1.matching_keys( probe );
  track_handle_list_type matching_activities;
  for (size_t i=0; i<a2a_keys.size(); ++i)
  {
    const track2track_type& key = a2a_keys[i];
    p1_probe_type p = activity_p1.t2t.find( key );
    if ( (p != activity_p1.t2t.end()) && ( ! p->second.frame_overlaps.empty() ))
    {
      track_handle_type other = (is_gt) ? key.second : key.first;
      matching_activities.push_back( other );
    }
  }

  // lookup the IDs, also lookup the source-track IDs
  vector< unsigned > matching_activity_ids;
  map< unsigned, bool > activity_source_track_id_map;
  for (size_t i=0; i<matching_activities.size(); ++i)
  {
    track_handle_type m = matching_activities[i];
    matching_activity_ids.push_back( id( m.row ));
    pair< bool, vector<unsigned> > s = source_track_ids.get( m.row );
    if ( ! s.first )
    {
      LOG_WARN( main_logger, "Process-single-match-stat for " << id( h.row ) << " : activity " <<
                id( m.row ) << " has no source track id?" );
      s.second = vector<unsigned>();
    }
    const vector<unsigned>& id_list = s.second;
    if ( id_list.size() != 1 )
    {
      LOG_WARN( main_logger, "Source_id_list contains " << id_list.size() << " elements; only processing first (if >1)" );
    }
    if ( id_list.size() >= 1)
    {
      activity_source_track_id_map[ id_list[0] ] = true;
    }
  }

  // output activity stanza

  {
    size_t n = matching_activity_ids.size();
    os << act_tag << " " << n << " ";
    for (size_t i=0; i<n; ++i)
    {
      os << matching_activity_ids[i] << " ";
    }
  }

  // do we have any activity-to-track matches that don't use the source track IDs?
  vector< track2track_type > a2t_keys = a2t_p1.matching_keys( probe );
  vector< unsigned > matching_track_ids;
  for (size_t i=0; i<a2t_keys.size(); ++i)
  {
    const track2track_type& key = a2t_keys[i];
    p1_probe_type p = a2t_p1.t2t.find( key );
    if ( (p != a2t_p1.t2t.end()) && ( ! p->second.frame_overlaps.empty() ))
    {
      track_handle_type other = (is_gt) ? key.second : key.first;
      // skip tracks associated with already output activities
      unsigned other_id = id( other.row );
      if ( activity_source_track_id_map.find( other_id ) ==
           activity_source_track_id_map.end() )
      {
        matching_track_ids.push_back( other_id );
      }
    }
  }

  // output track stanza

  {
    size_t n = matching_track_ids.size();
    os << ": " << track_tag << " " << n << " ";
    for (size_t i=0; i<n; ++i)
    {
      os << matching_track_ids[i] << " ";
    }
  }

  os << endl;
}


void
process_full_match_stats( const track2track_phase1& activity_p1,
                          const phase1_parameters& p1_params,
                          const track_handle_list_type& activity_truth_tracks,
                          const track_handle_list_type& activity_computed_tracks,
                          const track_handle_list_type& full_truth_tracks,
                          const track_handle_list_type& full_computed_tracks,
                          const string& match_fn )
{
  // Here's the matrix of possibilities:
  //
  //  \ G |  0  |  T   |   A
  // C \  |     |      |
  // -----+-----+------+-------
  //   0  |  1  |  2   |   3
  // -----+-----+------+-------
  //   T  |  4  |  5   |   6
  // -----+-----+------+-------
  //   A  |  7  |  8   |   9
  //
  // ...where:
  //
  //  'G' is the ground-truth
  //  'C' is the computed data
  //  'T' are moving object tracks
  //  'A' are activities
  //  '0' means there are no moving-objects or activities
  //
  // ...and the cells are interpreted as:
  //
  //  1 (G0,C0) : no ground-truth, no computed. True negatives.  (This
  //              cell is not directly observable and not computed.)
  //
  //  2 (GT,C0) : missed ground-truth track; ++FNt (false-negative-track)
  //
  //  3 (GA,C0) : missed ground-truth track, missed ground-truth activity.
  //
  //  4 (G0,CT) : false-positive track.
  //
  //  5 (GT,CT) : ground-truth track, computed track both present; ++tp(track)
  //
  //  6 (GA,CT) : got the ground-truth track, but missed the activity
  //
  //  7 (G0,CA) : computed track AND activity, both false
  //
  //  8 (GT,CA) : got the track, but false activity declared
  //
  //  9 (GA,CA) : got both the track and the activity
  //
  // score_tracks models cells 1,2,4,5.  Straight-up score_events only
  // models cells 0,3,7,9.  How do we cover cells 8 and 6?  For
  // example: ground-truth activity G has no match in the computed
  // activity set.  Is this cell 3 (tracking miss) or cell 6 (tracking
  // hit but activity miss)?
  //
  // 1) We convert activity G to source track G'
  // 2) We then compute an association matrix of the set of G's to all computed
  //    tracks.
  //
  // Likewise, we have a computed activity C with no matching ground-truth
  // activity.  How do we distinguish between cell 7 (false activity
  // on a tracking false alarm) or cell 8 (false activity on a true track?)
  // We convert C to its geometry C', and compute the association matrix
  // of the set of C's to all ground-truth tracks.
  //
  // In other words, we need THREE phase-1 objects:
  // 1) GA <-> CA : covers cells 1,7,3,9
  // 2) GA <-> CT : If (1) returns cell 3, distinguishes between cells 3 and 6
  // 3) GT <-> CA : If (1) returns cell 7, distinguishes ebtween cells 7 and 8
  //
  // ...in fact, since track_oracle activities always have their own geometry
  // (hello, KWE) we can skip the "convert activity to track" step.  Although
  // we do need some bookkeeping to avoid double-counting between cells 8/9
  // or 6/9.
  //
  // Note that since we never compute CT <-> GT, we don't compute cell 5 and
  // this is not a replacement for score_tracks.  On the other hand, we don't
  // have the time loss of computing CT <-> GT.
  //
  //
  // Here's what we'll output (one line, split up for clarity)
  //
  // {axis-tag} {activity-id}
  //    : {act-tag} {act-count} {act-id...}
  //    : {track-tag} {track-count} {track-id...}
  //
  // where:
  //
  //  'axis-tag' is either GA (ground-truth) or CA (computed) activity
  //
  //  'activity-id' is the ID of the activity
  //
  //  'act-tag' is either CA or GA (inverse sense of axis-tag)
  //
  //  'act-count' is the count of matching activities
  //
  //  'act-id' (one per act-count) is the ID of the matching activity
  //
  //  'track-tag' is either CT or GT (inverse sense of axis-tag, same
  //   sense as act-tag)
  //
  //  'track-count' is the count of  matching tracks (not counting matching
  //  tracks supporting matching activities)
  //
  //  'track-id' (one per track-count) is the ID of the matching track
  //
  // Some examples:
  //
  // GA 43 : CA 1 39 : CT 0
  //
  // ...ground-truth activity 43 has one computed activity match (#39)
  // [cell 9] and no other spurious matches.
  //
  // GA 13 : CA 0 : CT 2 191 193
  //
  // ...ground-truth activity 13 has no activity matches but 2 tracks
  // which match its geometry: tracks 191 and 193.  Cell 6.
  //
  // CA 8 : GA 0 : GT 0
  //
  // ...computed activity 8 has no matches in either activity or tracks.
  // Cell 7.
  //

  // set up

  ofstream os( match_fn.c_str() );
  if ( ! os )
  {
    LOG_ERROR( main_logger, "Couldn't open '" << match_fn << "' for writing; skipping match stat output" );
    return;
  }

  track2track_phase1 ga_to_ct_p1( p1_params ), ca_to_gt_p1( p1_params );
  LOG_INFO( main_logger, "Activity match stats: computing true activity -> computed track overlaps..." );
  ga_to_ct_p1.compute_all( activity_truth_tracks, full_computed_tracks );
  LOG_INFO( main_logger, "Activity match stats: computing computed activity -> true track overlaps..." );
  ca_to_gt_p1.compute_all( full_truth_tracks, activity_computed_tracks );


  // Generate the G lines

  for (size_t i=0; i<activity_truth_tracks.size(); ++i)
  {
    process_single_match_stat( os,
                               activity_truth_tracks[i],
                               /* is_gt = */ true,
                               activity_p1,
                               ga_to_ct_p1 );
  }

  // Generate the C lines
  for (size_t i=0; i<activity_computed_tracks.size(); ++i)
  {
    process_single_match_stat( os,
                               activity_computed_tracks[i],
                               /* is_gt = */ false,
                               activity_p1,
                               ca_to_gt_p1 );
  }

}

struct score_events_args_type
{
  struct kpf_context_t
  {
    enum class style_t {INVALID, OBJECT, ACTIVITY, ADHOC};

    style_t style;             // scoring for objects, activity, or an adhoc cset field
    string adhoc_field;        // if adhoc, cset field name
    string artifact;           // object or activity name
    int domain;                // the KPF domain we're scoring, or -1
    packet_header_t conf_src;  // KPF packet in computed data containing the confidence
    kpf_context_t(): style(style_t::INVALID) {}
  };

  vul_arg<bool> verbose_flag;
  vul_arg<bool> disable_sanity_checks_arg;
  vul_arg<string> activity_names_arg;
  vul_arg<unsigned> top_n_arg;
  vul_arg<bool> ct_in_rank_arg;
  vul_arg< string > t2t_dump_fn_arg;
  vul_arg< bool > disable_t2t_dump_cmd_file;
  vul_arg< string > task_arg;
  vul_arg< bool > convert_prob_to_relevancy_arg;
  vul_arg< bool > link_tracks_arg;
  vul_arg< int > max_n_roc_points_arg;
  vul_arg< bool > gt_prefiltered_arg;
  vul_arg< bool > ct_prefiltered_arg;
  vul_arg< string > track_style_arg;
  vul_arg< string > dump_filtered_gt_arg;
  vul_arg< string > dump_filtered_ct_arg;
  vul_arg< string > pvo_filename_arg;
  vul_arg< bool > display_git_hash_only_arg;
  vul_arg< string > kwe_gt_arg;
  vul_arg< string > kwe_ct_arg;
  vul_arg< string > kwe_track_style_arg;
  vul_arg< string > activity_match_arg;
  vul_arg< string > track_dump_fn_arg;

  vul_arg< string > kpf_target_arg;
  vul_arg< string > kpf_conf_src_arg;
  vul_arg< string > kpf_types_gt_arg;
  vul_arg< string > kpf_types_ct_arg;
  vul_arg< string > kpf_activity_gt_arg;
  vul_arg< string > kpf_activity_ct_arg;
  kpf_context_t kpf_context;


  score_events_args_type():
    verbose_flag( "-v", "dump more debugging information", false ),
    disable_sanity_checks_arg( "--disable-sanity-checks", "Check for basic track overlaps before scoring", false ),
    activity_names_arg( "--a", "comma-separated activities described by computed results (leave blank for list)" ),
    top_n_arg( "--top-n", "The top n events to use from the computed file. (unsigned) ('0' refers to using all events)", 0),
    ct_in_rank_arg( "--ct-in-rank", "Flag notifying that the computed tracks were already sorted by rank when written, so they will be in rank order when read in", false ),
    t2t_dump_fn_arg( "--t2t-dump-file", "dump track-to-track details here" ),
    disable_t2t_dump_cmd_file( "--t2t-disable-cmd-file", "set to disable default creation of 'plot-{t2t-dump-file}-cmd.txt'" ),
    task_arg( "--do", "tasks to do: 'roc', 'pr', 'pr,roc'", "roc" ),
    convert_prob_to_relevancy_arg( "--p2r", "copy probability score to relevancy to score on probability" ),
    link_tracks_arg( "--link", "link KWXML descriptor tracks based on source track ID" ),
    max_n_roc_points_arg( "--n-roc-points", "maxmimum number of roc points, -1 for all", -1 ),
    gt_prefiltered_arg( "--gt-prefiltered", "Set if ground truth tracks are pre-filtered for activity", false ),
    ct_prefiltered_arg( "--ct-prefiltered", "Set if computed tracks are pre-filtered for activity", false ),
    track_style_arg( "--kwxml_ts", "kwxml track style to copy", "PVMovementDescriptor" ),
    dump_filtered_gt_arg( "--dump-filtered-gt", "write final filtered ground-truth tracks as kwxml" ),
    dump_filtered_ct_arg( "--dump-filtered-ct", "write final filtered computed tracks as kwxml" ),
    pvo_filename_arg( "--pvo", "filename:{p,v,o} - read pvo values from file; use p,v, or o values; no activity required", "" ),
    display_git_hash_only_arg( "--git-hash", "Display git hash and exit", false),
    kwe_gt_arg( "--kwe-gt", "load KWE of gt events" ),
    kwe_ct_arg( "--kwe-ct", "load KWE of ct events" ),
    kwe_track_style_arg( "--kwe-track-style", "Set track style of events from KWE to this", "" ),
    activity_match_arg( "--activity-matches", "write activity match status here (increases run time) "),
    track_dump_fn_arg( "--write-tracks", "Write annotated input tracks to this file (either .kwcsv or .kwiver)" ),
    kpf_target_arg( "--kpf-target", "[object|activity]:$type:$domain, e.g 'object:person:2' or 'activity:walking:3'" ),
    kpf_conf_src_arg( "--kpf-conf-src", "KPF packet type / domain containing the confidence we're scoring, e.g. cset2" ),
    kpf_types_gt_arg( "--kpf-types-gt", "KPF types file for ground-truth" ),
    kpf_types_ct_arg( "--kpf-types-ct", "KPF types file for computed" ),
    kpf_activity_gt_arg( "--kpf-activity-gt", "KPF activity file for ground-truth" ),
    kpf_activity_ct_arg( "--kpf-activity-ct", "KPF activity file for computed" )
  {}

  // not const because many vul_arg methods aren't marked const

  bool set_kpf_context();

  bool sanity_check( input_args_type& input_args,
                     matching_args_type& matching_args );

  activity_selector_type deduce_activity();

  void process_track_dumps( const track_handle_list_type& truth_tracks,
                            const track_handle_list_type& computed_tracks ) const;

  bool filter_activity_tracks( const activity_selector_type& what_act,
                               track_handle_list_type& truth_tracks,
                               track_handle_list_type& computed_tracks );

  bool copy_kpf_activities( const track_handle_list_type& kpf_activities,
                            track_handle_list_type& geometry_tracks,
                            const activity_selector_type& what_act,
                            bool is_gt );

  bool set_kpf_object_types( const string& types_fn,
                             const activity_selector_type& what_act,
                             track_handle_list_type& tracks );

};

bool
score_events_args_type
::set_kpf_context()
{
  // only do anything if kpf target is set
  if (! this->kpf_target_arg.set()) return true;

  // expecting three colon-separated fields: string, string, integer
  kwiversys::RegularExpression re( "^([^:]+):([^:]+):([0-9]+)$" );
  // ...or two fields: fieldname, artifact
  kwiversys::RegularExpression re_adhoc( "^([^:]+):([^:]+)$" );

  if ( re.find( this->kpf_target_arg() ))
  {

    kpf_context_t& c = this->kpf_context;
    // first field must be either 'object' or 'activity'
    if ( re.match(1) == "object" )
    {
      c.style = kpf_context_t::style_t::OBJECT;
    }
    else if ( re.match(1) == "activity" )
    {
      c.style = kpf_context_t::style_t::ACTIVITY;
    }
    else
    {
      LOG_ERROR( main_logger, "KPF target parse error: first field '"
                 << re.match(1) << "' must be either 'object' or 'activity'");
      return false;
    }

    // just accept the second and third fields
    c.artifact = re.match(2);
    c.domain = std::stoi( re.match(3) );

    // need to know where the computed confidence comes from
    c.conf_src = kwiver::vital::kpf::packet_header_parser( this->kpf_conf_src_arg() );
    if ( c.conf_src.style == kwiver::vital::kpf::packet_style::INVALID )
    {
      LOG_ERROR( main_logger, "KPF confidence source error: couldn't parse '"
                 << this->kpf_conf_src_arg() << "' as a KPF packet header" );
      c.style = kpf_context_t::style_t::INVALID;
      return false;
    }
  }
  else if (re_adhoc.find( this->kpf_target_arg() ))
  {
    kpf_context_t& c = this->kpf_context;
    c.style = kpf_context_t::style_t::ADHOC;
    c.adhoc_field = re_adhoc.match(1);
    c.artifact = re_adhoc.match(2);
  }

  return true;
}

bool
score_events_args_type
::sanity_check( input_args_type& input_args,
                matching_args_type& matching_args )
{

  // can't set both kwe_gt and kpf data set
  if ( this->kwe_gt_arg.set() &&
       (this->kpf_activity_gt_arg.set() || this->kpf_types_gt_arg.set() ))
  {
    LOG_ERROR( main_logger, "Can't filter truth tracks by both KWE and KPF" );
    return false;
  }

  // can't set both kwe_ct and kpf_ct
  if ( this->kwe_ct_arg.set() &&
       (this->kpf_activity_ct_arg.set() || this->kpf_types_ct_arg.set() ))
  {
    LOG_ERROR( main_logger, "Can't filter computed tracks by both KWE and KPF" );
    return false;
  }

  // scoring KPF object detections requires detection mode
  if ( (this->kpf_context.style == kpf_context_t::style_t::OBJECT)
       && ( ! input_args.detection_mode() ))
  {
    LOG_ERROR( main_logger, "Scoring KPF objects requires --detection-mode" );
    return false;
  }

  // scoring KPF adhoc detections requires detection mode
  if ( (this->kpf_context.style == kpf_context_t::style_t::ADHOC)
       && ( ! input_args.detection_mode() ))
  {
    LOG_ERROR( main_logger, "Scoring KPF objects requires --detection-mode" );
    return false;
  }

  // makes no sense to set types except in object mode
  if ( (this->kpf_types_gt_arg.set() || this->kpf_types_ct_arg.set())
       && (this->kpf_context.style != kpf_context_t::style_t::OBJECT))
  {
    LOG_ERROR( main_logger, "Only set KPF object types when scoring objects" );
    return false;
  }

  // makes no sense to set activities except in activity mode
  if ( (this->kpf_activity_gt_arg.set() || this->kpf_activity_ct_arg.set())
       && (this->kpf_context.style != kpf_context_t::style_t::ACTIVITY))
  {
    LOG_ERROR( main_logger, "Only set KPF activity types when scoring activities" );
    return false;
  }

  // cannot use detection mode and match-frames-lower-bound
  if ( input_args.detection_mode() && matching_args.min_frames_arg.set() && (matching_args.min_frames_arg() != "1"))
  {
    LOG_ERROR( main_logger, "Cannot use --detection-mode with --match-frames-lower-bound set to anything other than 1" );
    return false;
  }

  // if the user requested annotated track output, make sure the format is unambiguous
  if ( this->track_dump_fn_arg.set() )
  {
    vector< file_format_enum > fmts = file_format_manager::globs_match( track_dump_fn_arg() );
    if ( fmts.size() != 1 )
    {
      LOG_ERROR( main_logger, "Track output filename matches " << fmts.size() << " formats; use either .kwcsv or .kwiver" );
      return false;
    }
  }

  if ( ( ! matching_args.sanity_check()) || ( ! matching_args.parse_min_frames_arg() ))
  {
    return false;
  }

  if ( this->activity_match_arg.set() && this->top_n_arg.set() )
  {
    // Setting top-n filters the set of tracks the ROC is computed
    // over; this would lead to misleading activity matching
    // statistics in the full activity match file. For example: if you
    // only compute a ROC on the top-10, and #11 is a hit, how would you
    // reconcile that in the stats file?

    LOG_ERROR( main_logger, "Currently cannot simultaneously set '" << this->activity_match_arg.option() << "' and '"
               << this->top_n_arg.option() << "'" );
    return false;
  }

  if ( this->activity_match_arg.set() &&
       ( ! ( this->kwe_gt_arg.set() && this->kwe_ct_arg.set() )))
  {
    // Computing the match status matrix requires a rational method to derive
    // the underlying moving-object track from the activity track.  For this first
    // pass, we only support the KWE hack.  Moving forward, we could also support
    // kwxml by extracting the trackObjectKitware tracks.

    LOG_ERROR( main_logger, "Currently can only compute activity match status with events via KWE; exiting" );
    return false;
  }

  // validate top-n arg before bothering to load tracks
  if ( this->top_n_arg.set() && ( this->top_n_arg() == 0))
  {
    LOG_INFO( main_logger, this->top_n_arg.option() << " cannot explicitly be set to 0; exiting");
    return false;
  }

  return true;

}

activity_selector_type
score_events_args_type
::deduce_activity()
{
  //
  // There are too many ways to indicate which activity we're scoring:
  //
  // 1) An old-style VIRAT activity via '--a'
  // 2) A PVO specifier.
  // 3) Asserting that all tracks are the (unspecified) activity via
  //    gt-prefiltered.
  // 4) Naming a KPF activity.
  //

  activity_selector_type ret;

  this->set_kpf_context();

  //
  // First check for PVO. If set() fails, it's an error. If it succeeds
  // and pvo_req is valid, then we're PVO; set and return. Otherwise,
  // continue deducing.
  //

  if ( ! ret.pvo_req.set( this->pvo_filename_arg() ))
  {
    return ret;
  }
  if ( ret.pvo_req.valid )
  {
    ret.style = activity_style::PVO;
    return ret;
  }

  //
  // If we're prefiltered, set the style and return.
  //

  if ( this->gt_prefiltered_arg() )
  {
    ret.style = activity_style::PREFILTERED;
    return ret;
  }

  //
  // KPF activity scoring
  //

  if (( this->kpf_context.style == kpf_context_t::style_t::ACTIVITY ) ||
      ( this->kpf_context.style == kpf_context_t::style_t::OBJECT ) )
  {
    ret.style =
      this->kpf_context.style == kpf_context_t::style_t::ACTIVITY
      ? activity_style::KPF_ACTIVITY
      : activity_style::KPF_OBJECT;
    ret.activity_name = this->kpf_context.artifact;
    ret.activity_domain = this->kpf_context.domain;
    ret.kpf_conf_src = this->kpf_context.conf_src;
    return ret;
  }

  //
  // KPF adhoc scoring
  //
  if ( this->kpf_context.style == kpf_context_t::style_t::ADHOC )
  {
    ret.style = activity_style::KPF_ADHOC;
    ret.activity_name = this->kpf_context.artifact;
    ret.kpf_adhoc_field = this->kpf_context.adhoc_field;
    return ret;
  }

  //
  // If nothing else fits, see if we can find a VIRAT activity
  //

  vector<int> activity_indices;
  bool dump_activity_list = true;
  if ( this->activity_names_arg.set() )
  {
    vector<string> activity_names;

    boost::split(activity_names, activity_names_arg(), boost::is_any_of(","), boost::token_compress_on);

    try
    {
      transform( activity_names.begin(),
                 activity_names.end(),
                 back_inserter( activity_indices ),
                 aries_interface::activity_to_index );
      dump_activity_list = false;
    }
    catch ( aries_interface_exception& e )
    {
      LOG_INFO( main_logger, "At least one of '" << activity_names_arg()
                << "' is not a valid activity name: " << e.what() );
    }

    if ( ! activity_indices.empty() )
    {
      ret.activity_index = activity_indices[0];
      ret.style = activity_style::VIRAT;
    }
  }
  if ( dump_activity_list )
  {
    ret.style = activity_style::VIRAT_AND_EXIT;
    const map< size_t, string >& i2a_map = aries_interface::index_to_activity_map();
    map<size_t, string>::const_iterator probe;
    for (probe = i2a_map.begin(); probe != i2a_map.end(); ++probe )
    {
      LOG_INFO( main_logger, probe->second << "");
    }
  }


  return ret;
}

void
score_events_args_type
::process_track_dumps( const track_handle_list_type& truth_tracks,
                       const track_handle_list_type& computed_tracks ) const
{
  //
  // if requested, write out the final filtered tracks
  //

  if ( this->dump_filtered_gt_arg.set() )
  {
    bool rc = file_format_manager::write( this->dump_filtered_gt_arg(), truth_tracks );
    if (rc)
    {
      LOG_INFO( main_logger, "Wrote " << truth_tracks.size() << " filtered truth tracks to " << this->dump_filtered_gt_arg() );
    }
    else
    {
      LOG_WARN( main_logger, "Couldn't write filtered truth tracks to " << this->dump_filtered_gt_arg() );
    }
  }

  if ( this->dump_filtered_ct_arg.set() )
  {
    bool rc = file_format_manager::write( this->dump_filtered_ct_arg(), computed_tracks );
    if (rc)
    {
      LOG_INFO( main_logger, "Wrote " << computed_tracks.size() << " filtered computed tracks to " << this->dump_filtered_ct_arg() );
    }
    else
    {
      LOG_WARN( main_logger, "Couldn't write filtered computed tracks to " << this->dump_filtered_ct_arg() );
    }
  }
}

bool
score_events_args_type
::copy_kpf_activities( const track_handle_list_type& kpf_activities,
                       track_handle_list_type& geometry_tracks,
                       const activity_selector_type& what_act,
                       bool is_gt )
{
  if (what_act.style != activity_style::KPF_ACTIVITY)
  {
    LOG_ERROR( main_logger, "Logic error: copying KPF activities for " << what_act << "?" );
    return false;
  }

  //
  // This is actually a mini-version of normalize_activity_tracks().
  //
  // We could try to defer this logic until then, but that would
  // mean passing track_filter_kpf_activity (rather than typical
  // geometry-bearing tracks) out of the filter_activity_tracks()
  // call into the main(). Better to quickly copy things here
  // and set the relevancy and so forth and just no-op out the
  // filter_on_track_style and normalize_activity_tracks for KPF.
  //

  field_handle_type fh = track_oracle_core::lookup_by_name( "kpf_activity_labels" );
  if (fh == kwiver::track_oracle::INVALID_FIELD_HANDLE)
  {
    LOG_ERROR( main_logger, "KPF activity normalization requested but kpf_activity_labels not present?" );
    return false;
  }

  track_handle_list_type ret;
  relevancy_extractor_type* r = nullptr;
  if ( is_gt )
  {
    // For ground-truth, if the tag isn't there, mark as absent
    r = new kpf_activity_extractor_type( fh, what_act.activity_name, /* zero if absent = */ false );
  }
  else if (what_act.style == activity_style::KPF_ACTIVITY)
  {
    // For computed, if the tag isn't there, assume the value is zero
    r = new kpf_activity_extractor_type( fh, what_act.activity_name, /* zero if absent = */ true );
  }
  else
  {
    LOG_ERROR( main_logger, "Logic error: no relevancy extractor for " << what_act );
    return false;
  }

  if ( ! r->valid() )
  {
    LOG_ERROR( main_logger, "Invalid KPF extractor for " << what_act );
    return false;
  }

  track_field< double > relevancy( "relevancy" );
  track_filter_kpf_activity act_schema;
  for (const auto& kpf_src: kpf_activities )
  {
    auto p = r->get( kpf_src.row );
    if (! p.first )
    {
      // no match
      continue;
    }
    else if (what_act.style == activity_style::KPF_ACTIVITY)
    {
      // for each of the actors, set the relevancy
      for (const auto& actor: act_schema( kpf_src ).actors() )
      {
        relevancy( actor.row ) = p.second;
        ret.push_back( actor );
      }
    }
    else
    {
      LOG_ERROR( main_logger, "logic error: no KPF filter extractor case" );
      return false;
    }
  }

  delete r;
  geometry_tracks = ret;
  return true;
}

bool
score_events_args_type
::filter_activity_tracks( const activity_selector_type& what_act,
                          track_handle_list_type& truth_tracks,
                          track_handle_list_type& computed_tracks )
{
  // If the user specified KWE events, insert them now
  if ( this->kwe_gt_arg.set() )
  {
#ifdef KWANT_ENABLE_KWE
    track_handle_list_type kwe_tracks;
    if ( ! track_filter_kwe_type::read( this->kwe_gt_arg(), truth_tracks, this->kwe_track_style_arg(), kwe_tracks ))
    {
      return false;
    }
    truth_tracks.insert( truth_tracks.end(), kwe_tracks.begin(), kwe_tracks.end() );
#else
    LOG_ERROR( main_logger, "KWE filtering requested but KWE support not yet ported" );
    return false;
#endif
  }
  if ( this->kwe_ct_arg.set() )
  {
#ifdef KWANT_ENABLE_KWE
    track_handle_list_type kwe_tracks;
    if ( ! track_filter_kwe_type::read( this->kwe_ct_arg(), computed_tracks, this->kwe_track_style_arg(), kwe_tracks ))
    {
      return false
    }
    computed_tracks.insert( computed_tracks.end(), kwe_tracks.begin(), kwe_tracks.end() );
#else
    LOG_ERROR( main_logger, "KWE filtering requested but KWE support not yet ported" );
    return false;
#endif
  }

  if (this->kpf_activity_gt_arg.set())
  {
    track_handle_list_type kpf_activities;
    if ( ! track_filter_kpf_activity::read( this->kpf_activity_gt_arg(),
                                            truth_tracks,
                                            this->kpf_context.domain,
                                            kpf_activities ))
    {
      return false;
    }
    size_t old_size = truth_tracks.size();
    if (! this->copy_kpf_activities( kpf_activities,
                                     truth_tracks,
                                     what_act,
                                     /*truth = */ true ) )
    {
      return false;
    }
    LOG_INFO( main_logger, "KPF filtering truth tracks: " << old_size
              << " tracks before; " << truth_tracks.size() << " tracks after" );
  }
  if (this->kpf_activity_ct_arg.set())
  {
    track_handle_list_type kpf_activities;
    if ( ! track_filter_kpf_activity::read( this->kpf_activity_ct_arg(),
                                            computed_tracks,
                                            this->kpf_context.domain,
                                            kpf_activities ))
    {
      return false;
    }
    size_t old_size = computed_tracks.size();
    if ( ! this->copy_kpf_activities( kpf_activities,
                                      computed_tracks,
                                      what_act,
                                      /* truth = */ false ) )
    {
      return false;
    }
    LOG_INFO( main_logger, "KPF filtering computed tracks: " << old_size
              << " tracks before; " << computed_tracks.size() << " tracks after" );
  }

  return true;
}

bool
score_events_args_type
::set_kpf_object_types( const string& types_fn,
                        const activity_selector_type& what_act,
                        track_handle_list_type& tracks )
{
  namespace KPF=::kwiver::vital::kpf;
  // load the YAML

  ifstream is( types_fn.c_str() );
  if (! is )
  {
    LOG_ERROR( main_logger, "Couldn't load types file '" << types_fn << "'" );
    return false;
  }
  KPF::kpf_yaml_parser_t parser( is );
  KPF::kpf_reader_t reader( parser );
  kwiver::logging_map_type wmsgs( main_logger, KWIVER_LOGGER_SITE );

  kwiver::track_oracle::track_field< kwiver::track_oracle::dt::tracking::external_id > id;
  kwiver::track_oracle::track_field< kwiver::track_oracle::dt::tracking::frame_number > frame_number;

  size_t track_c = 0;
  size_t det_c = 0;
  while (reader.next())
  {
    namespace KPFC = ::kwiver::vital::kpf::canonical;

    auto kpf_track_id_probe = reader.transfer_packet_from_buffer(
      KPF::packet_header_t( KPF::packet_style::ID, KPFC::id_t::TRACK_ID ));
    auto cset_probe = reader.transfer_packet_from_buffer(
      KPF::packet_header_t( KPF::packet_style::CSET, what_act.activity_domain ));

    if (! kpf_track_id_probe.first )
    {
      wmsgs.add_msg( "Couldn't find ID1 packet" );
      continue;
    }
    if (! cset_probe.first)
    {
      ostringstream oss;
      oss << "Couldn't find cset " << what_act.activity_domain << " packet";
      wmsgs.add_msg( oss.str() );
      continue;
    }

    // loop over all the tracks-- there may be duplicate instances of a track ID due to detection mode
    for (auto t: tracks )
    {
      auto id_probe = id.get( t.row );
      if ( ! id_probe.first )
      {
        LOG_ERROR( main_logger, "Track without a track ID?" );
        return false;
      }
      if ( id_probe.second != kpf_track_id_probe.second.id.d )
      {
        continue;
      }
      for (auto f: track_oracle_core::get_frames( t ))
      {
        kwiver::track_oracle::kpf_utils::add_to_row( wmsgs, f.row, cset_probe.second );
        ++det_c;
      }
      ++track_c;
    }
    reader.flush();
  }
  LOG_INFO( main_logger, "Set " << track_c << " objects / " << det_c << " detections from " << types_fn );
  if ( ! wmsgs.empty() )
  {
    LOG_INFO( main_logger, "Messages begin ");
    wmsgs.dump_msgs();
    LOG_INFO( main_logger, "Messages end" );
  }
  return true;
}


int main( int argc, char *argv[] )
{

  score_events_args_type scoring_args;
  input_args_type input_args;
  output_args_type output_args;
  matching_args_type matching_args;

  ostringstream arg_oss;
  for (int i=0; i<argc; ++i) arg_oss << argv[i] << " ";

  vul_arg_parse( argc, argv );

  //  LOG_INFO( main_logger, "GIT-HASH: " << VIDTK_GIT_VERSION );
  LOG_INFO( main_logger, "GIT-HASH: output not supported yet" );
  if (scoring_args.display_git_hash_only_arg())
  {
    return EXIT_SUCCESS;
  }

  activity_selector_type what_act = scoring_args.deduce_activity();
  LOG_INFO( main_logger, "Activity deduction: " << what_act );

  if ( ! scoring_args.sanity_check( input_args, matching_args ) )
  {
    return EXIT_FAILURE;
  }

  if ( what_act.style == activity_style::INVALID )
  {
    LOG_ERROR( main_logger, "Couldn't deduce an activity type; exiting" );
    return EXIT_FAILURE;
  }
  if ( what_act.style == activity_style::VIRAT_AND_EXIT )
  {
    return EXIT_SUCCESS;
  }

  // This is a good point to emit the command line for logging
  LOG_INFO( main_logger, "Command line:\n" << arg_oss.str() );

  // if the user specified radial_overlap, set input_arg's compute_mgrs_data flag
  if ( matching_args.radial_overlap() >= 0.0 )
  {
#ifdef KWANT_ENABLE_MGRS
    input_args.compute_mgrs_data = true;
#else
    LOG_ERROR( main_logger, "Radial overlap requested but MGRS support not yet ported" );
    return EXIT_FAILURE;
#endif
  }

  //
  // deal with the inputs to get two sets of tracks, one computed,
  // one ground-truth.  Each will have timestamps.
  //

  track_handle_list_type computed_tracks, truth_tracks;
  if ( ! input_args.process( computed_tracks, truth_tracks )) return EXIT_FAILURE;

  // remember the unfiltered tracks in case we need them for a full stats run
  track_handle_list_type unfiltered_truth_tracks = truth_tracks;
  track_handle_list_type unfiltered_computed_tracks = computed_tracks;

  //
  // filter_activity_tracks only applies when an external activity file (KWE, KPF)
  // carries information about whether tracks may have the target.
  //
  // We treat ground-truth differently from computed: in a set of 100 ground-truth
  // tracks, 50 of which are "{running: 1.0}", we keep those fifty and discard the
  // other 50. In a set of 100 computed tracks, 50 of which are "{running: 1.0}", we
  // keep them ALL, so that at p(running)==0.0, 100 tracks are eligble.
  //

  if (! scoring_args.filter_activity_tracks( what_act, truth_tracks, computed_tracks ))
  {
    return EXIT_FAILURE;
  }

  //
  // if KPF types are supplied, load them in
  //
  if ( (scoring_args.kpf_context.style == score_events_args_type::kpf_context_t::style_t::OBJECT)
       && scoring_args.kpf_types_gt_arg.set())
  {
    LOG_INFO( main_logger, "Setting ground-truth KPF objects from " << scoring_args.kpf_types_gt_arg() );
    if (! scoring_args.set_kpf_object_types( scoring_args.kpf_types_gt_arg(), what_act, truth_tracks ))
    {
      return EXIT_FAILURE;
    }
    LOG_INFO( main_logger, "Ground-truth objects complete" );
  }
  if ( (scoring_args.kpf_context.style == score_events_args_type::kpf_context_t::style_t::OBJECT)
       && scoring_args.kpf_types_ct_arg.set())
  {
    LOG_INFO( main_logger, "Setting computed objects from " << scoring_args.kpf_types_ct_arg() );
    if (! scoring_args.set_kpf_object_types( scoring_args.kpf_types_ct_arg(), what_act, computed_tracks ))
    {
      return EXIT_FAILURE;
    }
    LOG_INFO( main_logger, "Computed objects complete" );
  }

  // First: throw out all ground-truth tracks not matching activity_name_arg()
  // (since they'd be misses anyway.)

  if ( scoring_args.gt_prefiltered_arg() )
  {
    LOG_INFO( main_logger, truth_tracks.size() << " truth tracks prefiltered for activity");
  }
  else
  {
    track_handle_list_type filtered_tracks = filter_on_track_style( truth_tracks, scoring_args.track_style_arg );
    LOG_INFO( main_logger, "Truth track style filtering: " << truth_tracks.size() << " before; "
              << filtered_tracks.size() << " after" );
    truth_tracks = filtered_tracks;
  }

  // all truth tracks must be able to support normalization
  track_handle_list_type norm_gt = normalize_activity_tracks( truth_tracks,
                                                              /* input_is_gt = */ true,
                                                              scoring_args.gt_prefiltered_arg(),
                                                              scoring_args.convert_prob_to_relevancy_arg(),
                                                              what_act );
  LOG_INFO( main_logger, "Truth track activity normalization: " << truth_tracks.size() << " before; "
            << norm_gt.size() << " after" );


  // Second: filter the computed tracks
  if ( scoring_args.ct_prefiltered_arg() )
  {
    LOG_INFO( main_logger, computed_tracks.size() << " computed tracks prefiltered for activity" );
  }
  else
  {
    track_handle_list_type filtered_tracks = filter_on_track_style( computed_tracks, scoring_args.track_style_arg );
    LOG_INFO( main_logger, "Computed track style filtering: " << computed_tracks.size() << " before; "
              << filtered_tracks.size() << " after" );
    computed_tracks = filtered_tracks;
  }

  // all computed tracks must be able to support normalization
  track_handle_list_type norm_comp = normalize_activity_tracks( computed_tracks,
                                                                /* input_is_gt = */ false,
                                                                scoring_args.ct_prefiltered_arg(),
                                                                scoring_args.convert_prob_to_relevancy_arg(),
                                                                what_act );
  LOG_INFO( main_logger, "computed track activity normalization: " << computed_tracks.size() << " before; "
            << norm_comp.size() << " after" );

  truth_tracks = norm_gt;
  computed_tracks = norm_comp;

  scoring_args.process_track_dumps( truth_tracks, computed_tracks );

  //
  // need to consolidate normalization
  //

  double fa_norm = 1.0;
  if ( matching_args.radial_overlap() >= 0.0 )
  {
#ifdef KWANT_ENABLE_MGRS
    fa_norm = compute_normalization_factors( truth_tracks, computed_tracks );
#else
    LOG_ERROR( main_logger, "Radial overlap requested but MGRS support not yet ported" );
    return EXIT_FAILURE;
#endif
  }
  else if (input_args.detection_mode() )
  {
    track_field<kwiver::track_oracle::dt::tracking::timestamp_usecs> ts;
    std::set< kwiver::track_oracle::dt::tracking::timestamp_usecs::Type > ts_set;
    track_handle_list_type all_tracks( truth_tracks );
    all_tracks.insert( all_tracks.end(), computed_tracks.begin(), computed_tracks.end() );
    for (size_t i=0; i<all_tracks.size(); ++i)
    {
      frame_handle_list_type f = track_oracle_core::get_frames( all_tracks[i] );
      for (size_t j=0; j<f.size(); ++j)
      {
        ts_set.insert( ts( f[j].row ));
      }
    }
    fa_norm = static_cast<double>( ts_set.size() );
    LOG_INFO( main_logger, "FA normalization: detection mode; factor: " << fa_norm << " frames" );
  }
  else
  {
    LOG_INFO( main_logger, "FA normalization: none; factor: " << fa_norm );
  }


  phase1_parameters p1_params;
  p1_params.perform_sanity_checks = ( ! scoring_args.disable_sanity_checks_arg() );
  if ( ! p1_params.processMatchingArgs( matching_args ))
  {
    return EXIT_FAILURE;
  }

  if ( scoring_args.link_tracks_arg() )
  {
    if ( what_act.style != activity_style::VIRAT )
    {
      LOG_ERROR( main_logger, "kwxml linking requested but only available for VIRAT activities; selected " << what_act );
      return EXIT_FAILURE;
    }
    LOG_INFO( main_logger, "relinking kwxml...");
    int activity_index_to_relevancy =
      scoring_args.convert_prob_to_relevancy_arg()
      ? what_act.activity_index
      : -1;
    track_handle_list_type linked_tracks = relink_kwxml_on_source_ids( computed_tracks, activity_index_to_relevancy );
    LOG_INFO( main_logger, computed_tracks.size() << " tracks in; " << linked_tracks.size() << " out");
    computed_tracks = linked_tracks;
  }

  // everybody needs a phase1 object
  // if requested, filter out the top N tracks

  track_handle_list_type scored_computed_tracks =
    scoring_args.top_n_arg.set()
    ? process_top_n_tracks( computed_tracks, scoring_args.top_n_arg(), scoring_args.ct_in_rank_arg() )
    : computed_tracks;

  // compute the association matrix.  Note that activity tracks are not
  // typically subsets of the full track set.

  track2track_phase1 p1( p1_params );
  {
    track_handle_list_type filtered_truth, filtered_computed;
    p1_params.filter_track_list_on_aoi( truth_tracks, filtered_truth );
    p1_params.filter_track_list_on_aoi( scored_computed_tracks, filtered_computed );
    LOG_INFO( main_logger, "p1: AOI kept "
              << filtered_truth.size() << " of " << truth_tracks.size() << " truth tracks; "
              << filtered_computed.size() << " of " << scored_computed_tracks.size() << " computed tracks");
    truth_tracks = filtered_truth;
    scored_computed_tracks = filtered_computed;
  }

  if (input_args.detection_mode())
  {
    p1.compute_all_detection_mode( truth_tracks, scored_computed_tracks );
  }
  else
  {
    p1.compute_all( truth_tracks, scored_computed_tracks );
  }


  if ( scoring_args.task_arg().find( "roc" ) != string::npos )
  {
    // if requested, write the time-axis track overlap debug plots
    if ( scoring_args.t2t_dump_fn_arg.set() )
    {
      process_p1_debug_dump( p1, truth_tracks, computed_tracks, scoring_args.t2t_dump_fn_arg(), scoring_args.disable_t2t_dump_cmd_file() );
    }

    // if requested, dump the matches file
    if (output_args.matches_dump_fn.set())
    {
      track2track_phase2_hadwav p2( /* verbose flag = */ false );
      p2.compute( truth_tracks, scored_computed_tracks, p1 );
      ofstream os (output_args.matches_dump_fn().c_str() );
      if ( os )
      {
        p2.debug_dump( os );
      }
      else
      {
        LOG_ERROR( main_logger, "Couldn't dump match info to '" << output_args.matches_dump_fn() << "'" );
      }
    }

    // compute the actual ROC, on only the activity tracks

    compute_roc( p1, truth_tracks, computed_tracks, fa_norm, scoring_args.max_n_roc_points_arg(), output_args );


    // if requested, process full match stats
    bool full_match_stats_requested = scoring_args.activity_match_arg.set();

    if (full_match_stats_requested)
    {
      process_full_match_stats( p1,
                                p1_params,
                                truth_tracks,
                                computed_tracks,
                                unfiltered_truth_tracks,
                                unfiltered_computed_tracks,
                                scoring_args.activity_match_arg() );
    }
  }

  if ( scoring_args.task_arg().find( "pr" ) != string::npos )
  {
    compute_pr( p1, truth_tracks, computed_tracks, output_args );
  }

  if ( scoring_args.track_dump_fn_arg.set() )
  {
    track_handle_list_type all_tracks;
    all_tracks.insert( all_tracks.end(), truth_tracks.begin(), truth_tracks.end() );
    all_tracks.insert( all_tracks.end(), computed_tracks.begin(), computed_tracks.end() );
    bool rc = file_format_manager::write( scoring_args.track_dump_fn_arg(), all_tracks, kwiver::track_oracle::TF_INVALID_TYPE );
    LOG_INFO( main_logger, "Write returned " << rc );
  }

  //
  // all done!
  //

  return EXIT_SUCCESS;
}


map< double, bool >
generate_roc_thresholds( const track_handle_list_type& computed_tracks,
                         int max_n_roc_points,
                         output_args_type& output_args )
{
  track_field< double > relevancy( "relevancy" );
  map< double, bool > roc_threshold;

  if ( !output_args.thresholds_arg.set() )
  {
    double max_r = -1.0;
    double min_r = 1.0e6;
    for (unsigned i=0; i<computed_tracks.size(); ++i)
    {
      double r = relevancy( computed_tracks[i].row );
      roc_threshold[ r ] = true;
      if ( r > max_r ) max_r = r;
      if ( r < min_r ) min_r = r;
    }

    LOG_INFO( main_logger, computed_tracks.size() << " computed events have "
              << roc_threshold.size() << " unique thresholds");

    // Not that this has ever happened to me...
    if (roc_threshold.size() == 1)
    {
      map<double, bool>::const_iterator probe = roc_threshold.begin();
      if (probe->first == 0.0) {
        LOG_WARN( main_logger,"*");
        LOG_WARN( main_logger,"* The set of computed events all have probability 0.");
        LOG_WARN( main_logger,"* This can happen when the kwxml_ts specifies a descriptor" );
        LOG_WARN( main_logger,"* which doesn't actually compute the activity you've selected." );
        LOG_WARN( main_logger,"*");
      }
    }

    // probably a smoother way to do this
    if (max_n_roc_points == -1)
    {
      LOG_INFO( main_logger, "...keeping them all" );
    }
    else if (roc_threshold.size()  > static_cast<size_t>( max_n_roc_points ))
    {
      LOG_INFO( main_logger, "min / max relevancy " << min_r << " / " << max_r );
      const double r_range = max_r - min_r;
      const double max_threshold_gap = r_range / max_n_roc_points;
      map< double, bool > reduced_roc_threshold;
      for (map<double, bool>::const_iterator i = roc_threshold.begin(); i != roc_threshold.end(); ++i)
      {
        bool keep_this = false;
        if (reduced_roc_threshold.empty() )
        {
          keep_this = true;
        }
        else
        {
          double last_val = reduced_roc_threshold.rbegin()->first;
          if ( (i->first - last_val) > max_threshold_gap )
          {
            keep_this = true;
          }
        }
        if (keep_this)
        {
          reduced_roc_threshold[ i->first ] = true;
        }
      }

      roc_threshold = reduced_roc_threshold;

      LOG_INFO( main_logger, "Info: ...reduced to " << roc_threshold.size() << " unique thresholds");
    }

    // add an epsilon at the end
    roc_threshold[ max_r + 0.001 ] = true;
  }
  else
  {
    string thresholds = output_args.thresholds_arg();
    replace( thresholds.begin(), thresholds.end(), ':', ' ' );
    stringstream thresholds_str( thresholds );

    double min = 0.0;
    double max = 1.0;
    double step = 0.1;

    thresholds_str >> min;
    thresholds_str >> max;
    thresholds_str >> step;

    // Add an epsilon to help avoid rounding goofs.
    max += 1e-5;

    for ( double thresh = min; thresh <= max; thresh += step )
    {
      roc_threshold[ thresh ] = true;
    }
  }

  return roc_threshold;
}
void
compute_roc( const track2track_phase1& p1,
             const track_handle_list_type& truth_tracks,
             const track_handle_list_type& computed_tracks,
             double fa_norm,
             int max_n_roc_points,
             output_args_type& output_args )
{
  //
  // Here's our first plan for scoring viqui:
  // we're sweeping an ROC over the relevency; hit/miss status based
  // on whether the track intersects a ground-truth track whose activity
  // matches activity_name_arg().  Later, when we score subsets,
  // order based on relevency first, then instance_id.
  //


  // build the map of ROC thresholds
  map< double, bool > roc_threshold =
    generate_roc_thresholds( computed_tracks,
                             max_n_roc_points,
                             output_args );

  // iterate over thresholds, printing out ROC stats
  // Record which truth tracks each computed track matches
  map< track_handle_type, track_handle_list_type > computed_to_truth_match_list;
  for (size_t i=0; i<computed_tracks.size(); ++i)
  {
    track_handle_list_type matches;
    computed_to_truth_match_list[ computed_tracks[i] ] = matches;
  }

  map< track2track_type, track2track_score >::const_iterator c2t_probe;
  for (c2t_probe = p1.t2t.begin(); c2t_probe != p1.t2t.end(); ++c2t_probe )
  {
    const track_handle_type& t = c2t_probe->first.first;
    const track_handle_type& c = c2t_probe->first.second;
    map< track_handle_type, track_handle_list_type >::iterator match_probe = computed_to_truth_match_list.find( c );
    if (match_probe == computed_to_truth_match_list.end() )
    {
      LOG_DEBUG( main_logger, "ROC fallthrough" );
    }
    else
    {
      match_probe->second.push_back( t );
    }
  }

  ostringstream roc_dump_str;
  ostringstream roc_csv_dump_str;
  roc_csv_dump_str << "threshold, PD, FA, nMatches, TP, FP, TN, FN, matched, relevant, nTrueTracks, faNorm\n";

  track_field< double > relevancy( "relevancy" );
  vector< map<track_handle_type, track_handle_list_type>::const_iterator > matches_cache;
  vector< double > relevancy_cache;
  for (size_t i=0; i<computed_tracks.size(); ++i)
  {
    map< track_handle_type, track_handle_list_type >::const_iterator probe
      = computed_to_truth_match_list.find( computed_tracks[i] );
    if ( probe == computed_to_truth_match_list.end() )
    {
      throw runtime_error( "Lost computed track handle in computed_to_truth_match_list map?" );
    }
    matches_cache.push_back( probe );
    relevancy_cache.push_back( relevancy( computed_tracks[i].row ));
  }

  for ( map<double, bool>::const_iterator roc_it = roc_threshold.begin();
        roc_it != roc_threshold.end();
        ++roc_it )
  {
    double threshold = roc_it->first;

    map< oracle_entry_handle_type, bool> true_matches;

    unsigned tp=0, fp=0, fn=0, tn=0;
    for (unsigned i=0; i < computed_tracks.size(); ++i)
    {

      // Computed track[i] has two attributes:
      // - R (relevance): true if its relevancy >= threshold
      // - M (match): true if it matches a true track
      //
      // R  &  M  : true positive (but unique against truth tracks)
      // R  & !M  : false positive
      // !R &  M  : false negative
      // !R & !M  : true negative
      //
      // True positives are recorded by setting flags in the true_matches map,
      // to factor out the possibility of multiple computed tracks matching a single
      // truth track.

      // establish R: does this track have a relevancy over the threshold?
      bool r_flag = ( relevancy_cache[i] >= threshold );

      // establish M: did we match a true track?
      map< track_handle_type, track_handle_list_type >::const_iterator probe
        = matches_cache[ i ];

      const track_handle_list_type& matches = probe->second;
      bool m_flag = ( ! matches.empty());
      if (r_flag)
      {
        for (size_t j=0; j<matches.size(); ++j)
        {
          true_matches[ matches[j].row ] = true;
        }
      }

      // increment counters
      if      ( r_flag && m_flag )      ++tp;
      else if ( r_flag && (!m_flag))    ++fp;
      else if ( (!r_flag) && m_flag)    ++fn;
      else if ( (!r_flag) && (!m_flag)) ++tn;
    } // ...for all computed tracks

    // output
    unsigned nMatches = true_matches.size();
    double pd =
      truth_tracks.empty()
      ? 0.0
      : 1.0 * nMatches / truth_tracks.size();

    roc_dump_str << vul_sprintf("roc threshold = %e ; pd = %e ; fa = %-5u ; nMatches = %-5u ; tp = %-5u ; fp = %-5u ; tn = %-5u ; fn = %-5u ; matched = %-5u ; relevant = %-5u ; nTrueTracks = %-5u ; fa-norm = %e\n",
                                threshold, pd, fp, nMatches, tp, fp, tn, fn, (tp+fn), (tp+fp), truth_tracks.size(), fp/fa_norm );
    roc_csv_dump_str << threshold << ", " << pd << ", " << fp << ", " << nMatches << ", " << tp << ", " << fp << ", "
                     << tn << ", " << fn << ", " << (tp+fn) << ",  " << (tp+fp) << ", " << truth_tracks.size() << ", "
                     << (fp / fa_norm) << "\n";

    if (roc_it == roc_threshold.begin())
    {
      LOG_INFO( main_logger, "ROC: first line: " << roc_dump_str.str() );
    }
  } // ... for each roc threshold

  if ( ! output_args.console_dump_arg())
  {
    LOG_INFO( main_logger, "ROC: output not written to console (use --console to dump)" );
  }
  else
  {
    LOG_INFO( main_logger, "ROC:\n" << roc_dump_str.str() );
  }
  if (output_args.roc_dump_fn.set())
  {
    LOG_INFO( main_logger, "[score_events] Dumping roc data to '" << output_args.roc_dump_fn().c_str() << "'" );

    fstream dump_stream(output_args.roc_dump_fn().c_str(),
                            fstream::out);
    dump_stream << roc_dump_str.str();
    dump_stream.flush();
    dump_stream.close();
  }
  if (output_args.roc_csv_dump_fn.set())
  {
    ofstream ofs( output_args.roc_csv_dump_fn().c_str() );
    if ( ! ofs )
    {
      LOG_ERROR( main_logger, "Couldn't write to '" << output_args.roc_csv_dump_fn() << "'" );
    }
    else
    {
      LOG_INFO( main_logger, "[score_events] Writing ROC CSV to '" << output_args.roc_csv_dump_fn() << "'" );
      ofs << roc_csv_dump_str.str();
    }
  }
}


void
compute_pr( const track2track_phase1& p1,
            track_handle_list_type truth_tracks,
            track_handle_list_type computed_tracks,
            output_args_type& output_args )
{
  // always sort computed tracks
  sort( computed_tracks.begin(), computed_tracks.end(), compare_kst_track_handle);

  // recall is computed against truth tracks; precision against computed tracks.
  //
  // A set of computed tracks (however big) will partition the set of truth tracks
  // into true and false detections; td + fd == |truth_tracks|.  Multiple computed
  // tracks hitting a true track will only count as a single true detection.
  //
  // A set of true tracks will partition the set of computed tracks into true and
  // false positives; tp + fp == |computed_tracks|.
  //
  // precision = tp / (tp + fp)
  // recall = td / (td + fd)

  // At each step in the PR curve, either tp or fp goes up by 1.
  // However, each step in the PR curve is not guaranteed to increment either td or fd.

  track_kst_type kst_schema;
  unsigned tp=0, fp=0;
  double last_r = numeric_limits<double>::max();

  // compute true detections: as we loop over the computed tracks,
  // entries in this map will flip from false to true; at each
  // computed track, td = # of entries set to true.

  map< track_handle_type, bool > truth_track_hit_map;

  // increment td count only when we flip an entry from false to true
  // (we never flip them back from true to false)
  unsigned td=0;
  unsigned nTrue = truth_tracks.size();

  ostream* pr_os = &cout;
  if ( output_args.pr_dump_fn.set() )
  {
    pr_os = new ofstream( output_args.pr_dump_fn().c_str() );
    if ( ! (*pr_os))
    {
      LOG_ERROR( main_logger, "WARNING: Couldn't open '" << output_args.pr_dump_fn() << "' for writing; defaulting back to cout");
      delete pr_os;
      pr_os = &cout;
    }
  }

  typedef vector< track2track_type > matches_t;
  map< oracle_entry_handle_type, matches_t* > matches_map;
  vul_timer timer;

  LOG_INFO( main_logger, "PR: matches map setup...") ;
  for (map< track2track_type, track2track_score >::const_iterator i = p1.t2t.begin();
       i != p1.t2t.end();
       ++i)
  {
    if (timer.real() > 5 * 1000)
    {
      LOG_INFO( main_logger, "PR: matches map " << matches_map.size() << " of " << computed_tracks.size() );
      timer.mark();
    }
    oracle_entry_handle_type key = i->first.second.row;
    if (matches_map.find( key ) == matches_map.end() )
    {
      matches_map[ key ] = new matches_t;
    }
    matches_map[key]->push_back( i->first );
  }
  LOG_INFO( main_logger, "PR: Matches map complete; contains " << matches_map.size() << " entries");
  timer.mark();


  matches_t empty_matches;
  for (unsigned i=0; i<computed_tracks.size(); ++i)
  {
    if (timer.real() > 5 * 1000)
    {
      LOG_INFO( main_logger, "pr curve: " << i << " of " << computed_tracks.size() << "..." );
      timer.mark();
    }

    track_handle_type c = computed_tracks[i];
    double r = kst_schema( c ).relevancy();

    map< oracle_entry_handle_type, matches_t* >::const_iterator probe = matches_map.find( c.row );
    const matches_t& matches =
      probe == matches_map.end()
      ? empty_matches
      : *probe->second;

    bool matched_truth_track = false;
    for (unsigned j=0; j < matches.size(); ++j)
    {
      track_handle_type t = matches[j].first;

      matched_truth_track = true;
      if ( ! truth_track_hit_map[ t ] )
      {
        truth_track_hit_map[ t ] = true;
        ++td;
      }
    } // ...for all truth tracks

    if ( matched_truth_track ) ++tp; else ++fp;
    double prec =
      ( tp + fp ) == 0
      ? 0.0
      : 1.0 * tp / (tp + fp );
    double recall =
      nTrue == 0
      ? 0
      : 1.0 * td / nTrue;
    (*pr_os) << "PR-curve: " << i << " relevancy= " << r << " tp= " << tp << " fp= " << fp << " prec= " << prec << " td= " << td << " nTrue= " << nTrue << "  recall= " << recall << "\n";

    if (r > last_r)
    {
      LOG_INFO( main_logger, "Inverted relevancy!  last was " << last_r << " ; this was " << r << "");
    }
    last_r = r;

  }

  for (map< oracle_entry_handle_type, matches_t* >::const_iterator probe = matches_map.begin();
       probe != matches_map.end();
       ++probe)
  {
    delete probe->second;
  }

  LOG_INFO( main_logger, "Sample plot command:\nplot \"running-pr.dat\" using 16:10 w lp, \"\" using 16:10 every 10::10 with points ls 3 ps 3  t \"every 10th\"");

  if (output_args.pr_dump_fn.set())    // i.e. not set to cout
  {
    delete pr_os;
  }
}
