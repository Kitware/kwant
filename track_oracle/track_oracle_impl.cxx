/*ckwg +5
 * Copyright 2010-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_oracle_impl.h"
#include <stdexcept>
#include <limits>
#include <algorithm>

#include <typeinfo>

#include <vul/vul_timer.h>
#include <vul/vul_sprintf.h>

#include <vgl/vgl_box_2d.h>

#include <utilities/uuid_able.h>

#include <logger/logger.h>


using std::find;
using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::numeric_limits;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::runtime_error;
using std::string;
using std::vector;


#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_track_oracle_impl_cxx__
VIDTK_LOGGER("track_oracle_impl_cxx");

vidtk::track_oracle_impl* vidtk::track_oracle::impl = 0;


namespace // anon
{

//
// This data structure is used to map current data terms
// to CSV v1 types.

struct csv_v1_mapping_type
{
private:
  bool valid;

public:
  string current_name, v1_name;
  vidtk::field_handle_type fh;
  bool is_track_header; // true == track header, false == frame header
  bool emit_default;
  csv_v1_mapping_type( const string& c, const string& v, bool th, bool ed)
    : valid(true),current_name(c), v1_name(v), fh(vidtk::INVALID_FIELD_HANDLE), is_track_header(th), emit_default(ed)
  {}
  csv_v1_mapping_type()
    : valid(false)
  {}
  bool is_valid() const
  {
    return (this->valid && (this->fh != vidtk::INVALID_FIELD_HANDLE));
  }
  static map<vidtk::field_handle_type, vector<string> > remap_from_existing_headers(
    vector< csv_v1_mapping_type >& v1_types,
    bool is_track_header,
    const map< vidtk::field_handle_type, vidtk::element_store_base*>& element_pool) ;

};

//
// This routine takes an ordered vector of csv_v1_mapping_types as
// a lookup table, trawls through the given set of current_csv_headers
// and returns the equivalent (and probably smaller) set of v1 headers.
//
// v1_types is an ordered list of csv COLUMNS, not data terms.
// If it's present in v1_types, then it HAS to be emitted, even if it's
// not in the set of current_csv_headers.  (For example, if your CSV
// had world_x and world_y, but no world_z, we need to force world_z here.)
//


map<vidtk::field_handle_type, vector<string> >
csv_v1_mapping_type
::remap_from_existing_headers(
    vector< csv_v1_mapping_type >& v1_types,
    bool /*are_track_headers*/,
    const map<vidtk::field_handle_type, vidtk::element_store_base*>& element_pool )
{
  map< vidtk::field_handle_type, vector<string> > v1_headers;

  for (size_t i=0; i<v1_types.size(); ++i)
  {
    csv_v1_mapping_type& v1_col = v1_types[i];

    //
    // Plow through the element pool, looking for v1_col.current_name in the csv_headers
    //
    vidtk::field_handle_type found_fh = vidtk::INVALID_FIELD_HANDLE;
    for (map<vidtk::field_handle_type, vidtk::element_store_base*>::const_iterator p = element_pool.begin();
         (p != element_pool.end()) && (found_fh == vidtk::INVALID_FIELD_HANDLE);
         ++p)
    {
      const vector<string>& headers = p->second->csv_headers();
      if ( find( headers.begin(), headers.end(), v1_col.current_name ) != headers.end() )
      {
        found_fh = p->first;
      }
    }
    if ( found_fh == vidtk::INVALID_FIELD_HANDLE ) throw runtime_error( "v1-mapping couldn't find '"+v1_col.current_name+"'" );
    v1_col.fh = found_fh;
    v1_headers[ found_fh ].push_back( v1_col.v1_name );
  }
  return v1_headers;
}


} // anon

namespace vidtk
{

track_oracle_impl
::track_oracle_impl()
  : field_count(0), row_count(1000), last_domain_allocated( DOMAIN_ALL )
{
  // create some system tables: __parent_track, __frame_list
  {
    element_descriptor e(
      "__frame_list",
      "system field for frames associated with track",
      typeid( static_cast< frame_handle_list_type* >(0) ).name(),
      element_descriptor::SYSTEM );
    this->unlocked_create_element< frame_handle_list_type >( e );

  }
  {
    element_descriptor e(
      "__parent_track",
      "system field for frames to link back to their parent track",
      typeid( static_cast< oracle_entry_handle_type* >(0) ).name(),
      element_descriptor::SYSTEM );
    this->unlocked_create_element< oracle_entry_handle_type >( e );
  }
}

field_handle_type
track_oracle_impl
::unlocked_lookup_by_name( const string& name ) const
{
  map< string, field_handle_type >::const_iterator probe = this->name_pool.find( name );
  field_handle_type h =
    ( probe != this->name_pool.end() )
    ? probe->second
    : INVALID_FIELD_HANDLE;
  return h;
}

field_handle_type
track_oracle_impl
::lookup_by_name( const string& name ) const
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  return this->unlocked_lookup_by_name( name );
}

field_handle_type
track_oracle_impl
::lookup_required_field( const string& fn ) const
{
  map< string, field_handle_type >::const_iterator probe = this->name_pool.find( fn );
  if (probe == this->name_pool.end() ) throw runtime_error( "Lost required field '" + fn + "'" );
  return probe->second;
}

element_descriptor
track_oracle_impl
::get_element_descriptor( field_handle_type f ) const
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  map< field_handle_type, element_store_base* >::const_iterator probe = this->element_pool.find( f );
  return
    ( probe != this->element_pool.end() )
    ? probe->second->get_descriptor()
    : element_descriptor();
}

const element_store_base*
track_oracle_impl
::get_element_store_base( field_handle_type f ) const
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  map< field_handle_type, element_store_base* >::const_iterator probe = this->element_pool.find( f );
  return
    ( probe != this->element_pool.end() )
    ? probe->second
    : 0;
}

element_store_base*
track_oracle_impl
::get_mutable_element_store_base( field_handle_type f ) const
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  map< field_handle_type, element_store_base* >::const_iterator probe = this->element_pool.find( f );
  return
    ( probe != this->element_pool.end() )
    ? probe->second
    : 0;
}


bool
track_oracle_impl
::unlocked_field_has_row( oracle_entry_handle_type row, field_handle_type field ) const
{
  if ( field == INVALID_FIELD_HANDLE ) return false;
  map< field_handle_type, element_store_base* >::const_iterator probe = this->element_pool.find( field );
  if ( probe == this->element_pool.end() )
  {
    throw runtime_error( "Attempted field_has_row on non-existent field" );
  }
  return probe->second->exists( row );
}

bool
track_oracle_impl
::field_has_row( oracle_entry_handle_type row, field_handle_type field )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  return this->unlocked_field_has_row( row, field );
}

vector< field_handle_type >
track_oracle_impl
::fields_at_row( oracle_entry_handle_type row ) const
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  vector< field_handle_type > ret;
  for (map< field_handle_type, element_store_base* >::const_iterator i = this->element_pool.begin();
       i != this->element_pool.end();
       ++i)
  {
    if (i->second->exists( row ))
    {
      ret.push_back( i->first );
    }
  }
  return ret;
}


oracle_entry_handle_type
track_oracle_impl
::get_next_handle()
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  return ++this->row_count;
}

handle_list_type
track_oracle_impl
::get_domain( domain_handle_type domain )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  handle_list_type ret;

  //
  // the concept of returning ALL the handles is a little ill-defined,
  // for two reasons:
  // 1) Practically speaking, the data columns are templated, and we
  // don't have a generic "occupied" column.
  // 2) Semantically speaking, we still mix tracks and frames, which
  // probably isn't what the user expects.
  //
  // There are probably occasions when the user might want "ALL_TRACKS" or
  // "ALL_FRAMES", but we'll defer that to the magic day when we separate
  // tracks and frames.
  //
  // for now, we'll return an empty list for DOMAIN_ALL.

  if (domain != DOMAIN_ALL)
  {
    map< domain_handle_type, handle_list_type >::const_iterator i = this->domain_pool.find( domain );
    if ( i != this->domain_pool.end() )
    {
      ret = i->second;
    }
  }
  return ret;
}

domain_handle_type
track_oracle_impl
::lookup_domain( const string& domain_name, bool create_if_not_found )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  map< string, domain_handle_type >::iterator probe =  this->domain_names.find( domain_name );

  // return if found
  if (probe != this->domain_names.end())   return probe->second;

  // return invalid handle if not found and not asked to create
  if (! create_if_not_found )              return domain_handle_type();

  // otherwise, create it
  domain_handle_type h = ++this->last_domain_allocated;
  this->domain_names[ domain_name ] = h;
  this->domain_pool[ h ] = handle_list_type();
  return h;
}

domain_handle_type
track_oracle_impl
::create_domain( )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  // pre-incrementing ensures that DOMAIN_ALL can never be allocated
  domain_handle_type h = ++this->last_domain_allocated;
  this->domain_names[ vul_sprintf("anon_domain_%d", static_cast<int>( h )) ] = h;
  handle_list_type empty;
  this->domain_pool[ h ] = empty;
  return h;
}

domain_handle_type
track_oracle_impl
::create_domain( const handle_list_type& handles )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  // pre-incrementing ensures that DOMAIN_ALL can never be allocated
  domain_handle_type h = ++this->last_domain_allocated;
  this->domain_names[ vul_sprintf("anon_domain_%d", static_cast<int>( h )) ] = h;
  this->domain_pool[ h ] = handles;
  return h;
}

void
track_oracle_impl
::unlocked_remove_row( oracle_entry_handle_type row )
{
  for (map< field_handle_type, element_store_base* >::iterator i = this->element_pool.begin();
       i != this->element_pool.end();
       ++i)
  {
    i->second->remove( row );
  }
}

bool
track_oracle_impl
::is_domain_defined( const domain_handle_type& domain )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );

  return (this->domain_pool.find( domain ) != this->domain_pool.end() );
}

bool
track_oracle_impl
::release_domain( const domain_handle_type& domain )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );

  map< domain_handle_type, handle_list_type >::iterator probe = this->domain_pool.find( domain );
  if (probe == this->domain_pool.end())
  {
    LOG_ERROR( "release_domain called on un-allocated domain " << domain );
    return false;
  }

  field_handle_type flh = this->unlocked_lookup_by_name( "__frame_list" );
  if ( flh == INVALID_FIELD_HANDLE ) throw runtime_error( "Lost __frame_list" );
  const pair< map< oracle_entry_handle_type, frame_handle_list_type >*, frame_handle_list_type >& frame_list_lookup
    = this->lookup_table< frame_handle_list_type >( flh );
  const map<oracle_entry_handle_type, frame_handle_list_type>& frame_lists = *frame_list_lookup.first;

  handle_list_type rows_to_delete;

  // get the list of handles to be deleted
  handle_list_type handles = this->domain_pool[ domain ];

  for (size_t i=0; i<handles.size(); ++i)
  {
    rows_to_delete.push_back( handles[i] );
    // get the frame list (if any)
    map< oracle_entry_handle_type, frame_handle_list_type >::const_iterator frame_probe = frame_lists.find( handles[i] );
    if (frame_probe != frame_lists.end() )
    {
      const frame_handle_list_type& frames = frame_probe->second;
      for (size_t j=0; j<frames.size(); ++j)
      {
        rows_to_delete.push_back( frames[j].row );
      }
    }
  }

  // erase them
  for (size_t i=0; i<rows_to_delete.size(); ++i)
  {
    this->unlocked_remove_row( rows_to_delete[i] );
  }

  // erase handle list
  this->domain_pool.erase( domain );

  // erase domain name
  for (map< string, domain_handle_type >::iterator i = this->domain_names.begin();
       i != this->domain_names.end();
       ++i)
  {
    if (i->second == domain)
    {
      this->domain_names.erase( i );
      break;
    }
  }
  return true;
}


bool
track_oracle_impl
::add_to_domain( const handle_list_type& handles, const domain_handle_type& domain )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  map< domain_handle_type, handle_list_type >::iterator i = domain_pool.find( domain );
  if ( i == domain_pool.end() ) return false;
  i->second.insert( i->second.end(), handles.begin(), handles.end() );
  return true;
}

bool
track_oracle_impl
::add_to_domain( const track_handle_type& track, const domain_handle_type& domain )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  map< domain_handle_type, handle_list_type >::iterator i = domain_pool.find( domain );
  if ( i == domain_pool.end() ) return false;
  i->second.push_back( track.row );
  return true;
}

bool
track_oracle_impl
::set_domain( const handle_list_type& handles, domain_handle_type domain )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  map< domain_handle_type, handle_list_type >::iterator i = domain_pool.find( domain );
  if ( i == domain_pool.end() ) return false;
  i->second = handles;
  return true;
}

frame_handle_list_type
track_oracle_impl
::unlocked_get_frames( const track_handle_type& t )
{
  return this->unlocked_get_field<frame_handle_list_type>( t.row, this->lookup_required_field( "__frame_list" ));
}

frame_handle_list_type
track_oracle_impl
::get_frames( const track_handle_type& t )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  return this->unlocked_get_frames( t );
}

void
track_oracle_impl
::set_frames( const track_handle_type& t, const frame_handle_list_type& frames )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  this->unlocked_get_field< frame_handle_list_type >( t.row, this->lookup_required_field( "__frame_list" )) = frames;
}

size_t
track_oracle_impl
::get_n_frames( const track_handle_type& t )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  return this->unlocked_get_field< frame_handle_list_type >( t.row, this->lookup_required_field( "__frame_list" )).size();
}

///
/// At the moment, XML output is mostly delegated to routines
/// in xml_output_common_classes.cxx in a rather clumsy fashion
/// using type (string, unsigned, double...) as a first index
/// and element_descriptor as a second index.  The exceptions are
/// here, where we output each track header and its frames.
///

///
/// The xml_output_helper is basically a collection of functions
/// offloaded here to avoid cluttering the track_oracl_impl interface
/// while allowing access to its unlocked access functions behind the
/// API mutex.
///

struct xml_output_helper
{
  explicit xml_output_helper( track_oracle_impl& t );
  void reset( const track_handle_type& t );
  bool set_frame_stats( const frame_handle_list_type& frames );
  void emit_normal_track_header( ostream& os );
  void emit_e2at_track_header( ostream& os );
  bool skip_this( map< field_handle_type, element_store_base* >::const_iterator j,
                  oracle_entry_handle_type row );

  track_oracle_impl& oracle;
  track_handle_type current_track;

  map< field_handle_type, bool > skip_list;
  field_handle_type frame_number_h;
  field_handle_type external_id_h;
  field_handle_type timestamp_h;
  field_handle_type box_h;
  field_handle_type type_h;
  pair< bool, unsigned > external_id_pair;

  field_handle_type start_time_secs_h;
  field_handle_type basic_annotation_h;
  field_handle_type augmented_annotation_h;

  unsigned min_frame_num;
  unsigned max_frame_num;
  bool has_fn;
  size_t no_fn_count;

  // set in reset()
  bool is_e2at;

};

xml_output_helper
::xml_output_helper( track_oracle_impl& t)
  : oracle( t )
{
  frame_number_h = oracle.unlocked_lookup_by_name( "frame_number" );
  external_id_h = oracle.unlocked_lookup_by_name( "external_id" );
  timestamp_h = oracle.unlocked_lookup_by_name( "timestamp_usecs" );
  box_h = oracle.unlocked_lookup_by_name( "bounding_box" );
  type_h = oracle.unlocked_lookup_by_name( "type" );

  // these are specially output for old VIRAT kwxml tracks
  skip_list[ frame_number_h ] = true;
  skip_list[ external_id_h ] = true;
  skip_list[ timestamp_h ] = true;
  skip_list[ box_h ] = true;
  skip_list[ type_h ] = true;

  // used in e2at output
  start_time_secs_h = oracle.unlocked_lookup_by_name( "start_time_secs" );
  basic_annotation_h = oracle.unlocked_lookup_by_name( "basic_annotation" );
  augmented_annotation_h = oracle.unlocked_lookup_by_name( "augmented_annotation" );
}

void
xml_output_helper
::reset( const track_handle_type& t )
{
  current_track = t;
  // our heuristic: it's an e2at callout if it has no frames,
  // at least one of the two callout fields, and a starting timestamp
  size_t annotation_count = 0;

  bool no_frames = oracle.unlocked_get_field< frame_handle_list_type >( t.row, oracle.lookup_required_field( "__frame_list" )).empty();
  bool has_starting_timestamp = (start_time_secs_h != INVALID_FIELD_HANDLE) && oracle.unlocked_field_has_row( t.row, start_time_secs_h );
  if ( (basic_annotation_h != INVALID_FIELD_HANDLE) && oracle.unlocked_field_has_row( t.row, basic_annotation_h ))     ++annotation_count;
  if ( (augmented_annotation_h != INVALID_FIELD_HANDLE) && oracle.unlocked_field_has_row( t.row, augmented_annotation_h )) ++annotation_count;

  is_e2at = no_frames && has_starting_timestamp && (annotation_count > 0);
}

bool
xml_output_helper
::set_frame_stats( const frame_handle_list_type& frames )
{
  // first pass through the frames to determine min/max frame numbers.
  // (Alternative is to run through it once but buffer into a stringstream,
  // which seems wasteful.)

  min_frame_num = numeric_limits<unsigned>::max();
  max_frame_num = numeric_limits<unsigned>::min();
  has_fn = false;
  no_fn_count = 0;

  for (size_t j=0; j<frames.size(); ++j)
  {
    oracle_entry_handle_type r = frames[j].row;
    if ( ! oracle.unlocked_field_has_row( r, frame_number_h ))
    {
      ++no_fn_count;
      continue;
    }
    has_fn = true;
    unsigned fn = oracle.unlocked_get_field< unsigned >( r, frame_number_h );
    min_frame_num = min( fn, min_frame_num );
    max_frame_num = max( fn, max_frame_num );
  }

  external_id_pair = make_pair( false, 0 );
  if (oracle.unlocked_field_has_row( current_track.row, external_id_h ))
  {
    external_id_pair.first = true;
    external_id_pair.second = oracle.unlocked_get_field< unsigned >( current_track.row, external_id_h );
  }

  if ( ! has_fn )
  {
    ostringstream id_str;
    if ( external_id_pair.first ) id_str << external_id_pair.second; else id_str << "(no-external-id-set)";
    LOG_ERROR( "Track ID " << id_str.str() << " has " << frames.size() << " frames but no frame numbers; skipping..." );
    return false;
  }
  if ( no_fn_count > 0 )
  {
    ostringstream id_str;
    if ( external_id_pair.first ) id_str << external_id_pair.second; else id_str << "(no-external-id-set)";
    LOG_WARN( "Track ID " << id_str.str() << " has " << no_fn_count << " of " << frames.size()
              << " frames without frame numbers; these are skipped" );
  }
  return true;
}

void
xml_output_helper
::emit_normal_track_header( ostream& os )
{
  // first, emit the track line: id, start / last frame numbers
  os << "<track startFrame=\"" << min_frame_num << "\" lastFrame=\"" << max_frame_num;
  if (external_id_pair.first) os << "\" id=\"" << external_id_pair.second << "\" ";
  os << " frameNumberOrigin=\"clip\">\n";
}

void
xml_output_helper
::emit_e2at_track_header( ostream& os )
{
  os << "<track>\n";
}

bool
xml_output_helper
::skip_this( map< field_handle_type, element_store_base* >::const_iterator j,
             oracle_entry_handle_type row )
{
  if (skip_list[ j->first ]) return true;
  if ( ! j->second->exists( row )) return true;
  return false;
}

bool
track_oracle_impl
::write_kwxml( ostream& os, const track_handle_list_type& tracks )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  xml_output_helper helper( *this );

  os << "<vibrantDescriptors>\n"
     << "<!--\n"
     << "  File of " << tracks.size() << " tracks emitted by track_oracle's kwxml writer\n"
     << "-->\n";

  vul_timer timer;

  for (size_t i=0; i<tracks.size(); ++i)
  {
    if ( timer.real() > 5 * 1000 )
    {
      ostringstream oss;
      oss << "XML: wrote " << i << " of " << tracks.size() << " tracks ("
          << vul_sprintf( "%02.2f%%", 100.0*i/tracks.size() ) << ")";
      LOG_INFO( oss.str() );
      timer.mark();
    }
    const track_handle_type t = tracks[i];

    frame_handle_list_type frames = this->unlocked_get_frames( t );

    helper.reset( t );

    if ( ! helper.is_e2at )
    {
      if (! helper.set_frame_stats( frames ) ) continue;

      helper.emit_normal_track_header( os );
    }
    else
    {
      helper.emit_e2at_track_header( os );
    }


    // go through all the other fields we know about and emit them, unless
    // they're system fields or on the skip list (i.e. already emitted above)

    for (map< field_handle_type, element_store_base* >::const_iterator j = this->element_pool.begin();
         j != this->element_pool.end();
         ++j)
    {
      if ( helper.skip_this( j, t.row )) continue;

      // need to look at element_descriptor to see if this is a system field,
      // e.g. __frame_list
      const element_descriptor& ed = j->second->get_descriptor();
      if ( ed.role == element_descriptor::SYSTEM ) continue;

      // emit it
      j->second->emit_as_XML( os, t.row );
    }

    for (size_t j=0; j<frames.size(); ++j)
    {
      oracle_entry_handle_type r = frames[j].row;
      if ( ! this->unlocked_field_has_row( r, helper.frame_number_h ))
      {
        continue;
      }
      unsigned fn = this->unlocked_get_field< unsigned >( r, helper.frame_number_h );

      os << "<bbox frame=\"" << fn << "\" ";

      // bounding box
      if ( this->unlocked_field_has_row( r, helper.box_h ))
      {
        vgl_box_2d< double > box = this->unlocked_get_field< vgl_box_2d< double > >( r, helper.box_h );
        os << "ulx=\"" << box.min_x() << "\" uly=\"" << box.min_y()
           << "\" lrx=\"" << box.max_x() << "\" lry=\"" << box.max_y() << "\" ";
      }

      // timestamp_usecs
      if ( this->unlocked_field_has_row( r, helper.timestamp_h ))
      {
        os << "timestamp=\""
           << this->unlocked_get_field< unsigned long long >( r, helper.timestamp_h )
           << "\" ";
      }

      // all done with this track's frames
      os << "/>\n";

    } // ... for all frames

    // all done with this track
    os << "</track>\n";

  } // ...for all tracks

  // all done!
  os << "</vibrantDescriptors>\n";
  return true;
}

bool
track_oracle_impl
::clone_nonsystem_fields( const oracle_entry_handle_type& src,
                          const oracle_entry_handle_type& dst )
{
  if ( src == INVALID_ROW_HANDLE )
  {
    LOG_ERROR( "clone_nonsystem_fields: uninitialized source" );
    return false;
  }
  if ( dst == INVALID_ROW_HANDLE )
  {
    LOG_ERROR( "clone_nonsystem_fields: uninitialized destination" );
    return false;
  }

  boost::unique_lock< boost::mutex > lock( this->api_lock );
  bool all_okay = true;
  for ( map< field_handle_type, element_store_base* >::iterator i = this->element_pool.begin();
        i != this->element_pool.end();
        ++i )
  {
    const element_descriptor& ed = i->second->get_descriptor();
    if ( ed.role == element_descriptor::SYSTEM ) continue;
    if ( ! i->second->exists( src )) continue;
    all_okay &= i->second->copy_value( src, dst );
  }
  return all_okay;
}

void
track_oracle_impl
::emit_pool_as_kwiver( ostream& os, const string& indent, oracle_entry_handle_type row ) const
{
  for (map< field_handle_type, element_store_base* >::const_iterator j = this->element_pool.begin();
       j != this->element_pool.end();
       ++j)
  {
    // need to look at element_descriptor to see if this is a system field,
    // e.g. __frame_list
    const element_descriptor& ed = j->second->get_descriptor();
    if ( ed.role == element_descriptor::SYSTEM ) continue;

    // emit it
    j->second->emit_as_kwiver( os, row, indent );
  }
}

void
track_oracle_impl
::emit_pool_as_csv( ostream& os,
                    const vector< csv_element >& output_order,
                    oracle_entry_handle_type row,
                    size_t sequence_number,
                    bool is_track,
                    bool csv_v1_semantics ) const
{
  field_handle_type external_id_fh = this->unlocked_lookup_by_name( "external_id" );
  if ( external_id_fh == INVALID_FIELD_HANDLE ) throw runtime_error( "emit-csv lost external ID column?" );

  size_t n = output_order.size();
  for (size_t i=0; i<n; ++i)
  {
    field_handle_type target_fh = output_order[i].fh;
    if ( target_fh == INVALID_FIELD_HANDLE )
    {
      if ( output_order[i].is_track == is_track )
      {
        os << sequence_number;
      }
    }
    else
    {
      map< field_handle_type, element_store_base* >::const_iterator p = this->element_pool.find( target_fh );
      if (p->first == INVALID_FIELD_HANDLE )
      {
        if ( p == this->element_pool.end() ) throw runtime_error( "Lost a data column writing CSV" );
      }

      oracle_entry_handle_type emitted_row = row;

      if ( (! is_track)
           && csv_v1_semantics
           && (target_fh == external_id_fh))
      {
        // all this because we don't have a const lookup?
        field_handle_type pt_fh = this->unlocked_lookup_by_name( "__parent_track" );
        map< field_handle_type, element_store_base* >::const_iterator pt_probe = this->element_pool.find( pt_fh );
        element_store<oracle_entry_handle_type>* es_ptr = dynamic_cast< element_store<oracle_entry_handle_type>*>( pt_probe->second );
        if ( ! es_ptr ) throw runtime_error ("Bad column cast looking for external id" );
        map< oracle_entry_handle_type, oracle_entry_handle_type >::const_iterator pt_row_probe = es_ptr->storage.find( row );
        if ( pt_row_probe == es_ptr->storage.end() ) throw runtime_error( "Frame has no parent track?" );
        emitted_row = pt_row_probe->second;
      }

      p->second->emit_as_csv( os, emitted_row, output_order[i].emit_default_if_absent );
    }

    if ( i < n-1 )
    {
      os << ",";
    }
  }

  os << "\n";
}


bool
track_oracle_impl
::write_kwiver( ostream& os, const track_handle_list_type& tracks )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );
  os << "<kwiver>\n"
     << "<!--\n"
     << "  File of " << tracks.size() << " tracks emitted by track_oracle's kwiver writer\n"
     << "-->\n";

  const string indents[] = {
    "  ",
    "    ",
    "      "
  };

  for (size_t i=0; i<tracks.size(); ++i)
  {
    const track_handle_type t = tracks[i];

    frame_handle_list_type frames = this->unlocked_get_frames( t );
    os << indents[0] << "<track index=\"" << i << "\" len=\"" << frames.size() << "\" >\n";

    this->emit_pool_as_kwiver( os, indents[1], t.row );

    for (size_t j=0; j<frames.size(); ++j)
    {
      const frame_handle_type f = frames[j];
      os << indents[1] << "<frame index=\"" << j << "\" >\n";

      this->emit_pool_as_kwiver( os, indents[2], f.row );

      os << indents[1] << "</frame>\n";
    }
    os << indents[0] << "</track>\n\n";
  }
  os << "</kwiver>\n";
  return true;
}

void
track_oracle_impl
::get_csv_columns( const oracle_entry_handle_type& row,
                   map< field_handle_type , vector<string> >& m ) const
{
  for ( map<field_handle_type, element_store_base* >::const_iterator i = this->element_pool.begin();
        i != this->element_pool.end();
        ++i )
  {
    // need to look at element_descriptor to see if this is a system field,
    // e.g. __frame_list
    const element_descriptor& ed = i->second->get_descriptor();
    if ( ed.role == element_descriptor::SYSTEM ) continue;

    // do we have a value here?
    if ( ! i->second->exists( row )) continue;

    m[ i->first ] = i->second->csv_headers();
  }
}


bool
track_oracle_impl
::write_csv( ostream& os, const track_handle_list_type& tracks, bool csv_v1_semantics )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );

  field_handle_type world_gcs_fh = this->unlocked_lookup_by_name( "world_gcs" );
  if ( world_gcs_fh == INVALID_FIELD_HANDLE )
  {
    throw runtime_error( "world_gcs not registered" );
  }

  typedef map< field_handle_type, vector<string> >::const_iterator header_cit;
  typedef map< field_handle_type, vector<string> >::iterator header_it;

  // first, zip down the tracks and build up the track and frame header list

  map< field_handle_type, vector<string> > track_headers, frame_headers;

  // insert CSV bookkeeping fields
  // string values also used in file_format_csv's reader

  track_headers[ INVALID_FIELD_HANDLE ].push_back( "_track_sequence" );
  frame_headers[ INVALID_FIELD_HANDLE ].push_back( "_parent_track" );

  vul_timer timer;

  for ( size_t i=0; i<tracks.size(); ++i )
  {
    if ( timer.real() > 5 * 1000 )
    {
      ostringstream oss;
      oss << "CSV: Scanned " << i << " of " << tracks.size();
      LOG_INFO( oss.str() );
      timer.mark();
    }
    this->get_csv_columns( tracks[i].row, track_headers );
    frame_handle_list_type frames = this->unlocked_get_frames( tracks[i] );
    for ( size_t j=0; j<frames.size(); ++j )
    {
      this->get_csv_columns( frames[j].row, frame_headers );
    }
  }

  //
  // SPECIAL CASE:
  // Duplicating Matthew's file_format_csv logic, add world_gcs if
  // world_x and world_y are being set
  //
  // For now, hammer the names in: ideally, track_oracle_impl doesn't
  // know about data_terms.  Ideally, kwiver and csv I/O are routines
  // outside the impl which can know about them, but we'll fix that
  // when we get rid of the old legacy VIRAT xml output.
  //
  // If you write out a file, and it inserts world_gcs, and then read
  // the file, world_gcs will appear in the track headers because we
  // write out default values on both track and frames.
  //

  if ( (frame_headers.find( this->unlocked_lookup_by_name( "world_x" )) != frame_headers.end())
       &&
       (frame_headers.find( this->unlocked_lookup_by_name( "world_y" )) != frame_headers.end()) )
  {
    bool gcs_in_frame = frame_headers.find( world_gcs_fh ) != frame_headers.end();
    bool gcs_in_track = track_headers.find( world_gcs_fh ) != track_headers.end();
    if ( ! ( gcs_in_frame || gcs_in_track ))
    {
      frame_headers[ world_gcs_fh ] = this->element_pool[ world_gcs_fh ]->csv_headers();
    }
  }

  // another special case for world_gcs: if it's defined on the track, delete that,
  // so we get it on the frame.
  if (track_headers.find( world_gcs_fh ) != track_headers.end() )
  {
    track_headers.erase( world_gcs_fh );
  }

  // de-duplicate between track and frame; when in both, prefer track
  for ( header_it i = track_headers.begin(); i != track_headers.end(); ++i)
  {
    // skip INVALID_FIELD_HANDLEs
    if (i->first == INVALID_FIELD_HANDLE) continue;

    if (frame_headers.find( i->first ) != frame_headers.end() )
    {
      frame_headers.erase( i->first);
    }
  }

  //
  // Special csv-v1 logic
  //

  vector< csv_element  > output_order;

  if ( csv_v1_semantics )
  {
    //external_id,unique_id,frame_number,timestamp_usecs,obj_x,obj_y,obj_bbox_ul_x,obj_bbox_ul_y,obj_bbox_lr_x,obj_bbox_lr_y,world_x,world_y,world_z,world_gcs,velocity_x,velocity_y
    vector< csv_v1_mapping_type > v1_order;

    const bool is_track_header = true, is_frame_header = false;
    const bool YES_EMIT_DEFAULT = true, no_emit_default = false;

    v1_order.push_back( csv_v1_mapping_type( "external_id", "external_id", is_track_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "track_uuid", "unique_id", is_track_header, YES_EMIT_DEFAULT ));
    v1_order.push_back( csv_v1_mapping_type( "frame_number", "frame_number", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "timestamp_usecs", "timestamp_usecs", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "obj_location_x", "obj_x", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "obj_location_y", "obj_y", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "bounding_box_ul_x", "obj_bbox_ul_x", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "bounding_box_ul_y", "obj_bbox_ul_y", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "bounding_box_lr_x", "obj_bbox_lr_x", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "bounding_box_lr_y", "obj_bbox_lr_y", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "world_x", "world_x", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "world_y", "world_y", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "world_z", "world_z", is_frame_header, YES_EMIT_DEFAULT ));
    v1_order.push_back( csv_v1_mapping_type( "world_gcs", "world_gcs", is_frame_header, YES_EMIT_DEFAULT ));
    v1_order.push_back( csv_v1_mapping_type( "velocity_x", "velocity_x", is_frame_header, no_emit_default ));
    v1_order.push_back( csv_v1_mapping_type( "velocity_y", "velocity_y", is_frame_header, no_emit_default ));

    map< field_handle_type, vector<string> > new_track_headers =
      csv_v1_mapping_type::remap_from_existing_headers(
        v1_order,
        /* is_track = */ true,
        this->element_pool );

    map< field_handle_type, vector<string> > new_frame_headers =
      csv_v1_mapping_type::remap_from_existing_headers(
        v1_order,
        /* is_track = */ false,
        this->element_pool );

    // the field handles should now be initialized; we can set the output order.

    track_headers.clear();
    frame_headers.clear();
    for (size_t i=0; i<v1_order.size(); ++i)
    {
      csv_v1_mapping_type& this_col = v1_order[i];
      if ( ! this_col.is_valid() )
      {
        LOG_WARN( "CSV version 1: Column '" << this_col.v1_name << "' invalid; will not be emitted" );
        continue;
      }

      bool emit_default = ((this_col.v1_name == "world_gcs") || (this_col.v1_name == "unique_id" ));

      //
      // output_order should only contain ONE instance of each field handle.
      //

      bool is_fh_present = false;
      for (size_t j=0; j<output_order.size(); ++j)
      {
        if (output_order[j].fh == this_col.fh )
        {
          is_fh_present = true;
        }
      }

      if ( ! is_fh_present )
      {
        output_order.push_back(
          csv_element(
            this_col.fh,
            emit_default,
            this_col.is_track_header ));
      }

      if (this_col.is_track_header)
      {
        track_headers[ this_col.fh ] = new_track_headers[ this_col.fh ];
      }
      else
      {
        frame_headers[ this_col.fh ] = new_frame_headers[ this_col.fh ];
      }
    }
  }
  else
  {

    // non-csv-v1 (i.e. current, auto-adjusting) CSV
    // output order is lexigraphical order
    for ( header_cit i=track_headers.begin(); i != track_headers.end(); ++i )
    {
      output_order.push_back( csv_element( i->first, i->first == world_gcs_fh, /* is_track = */ true ) );
    }
    for ( header_cit i=frame_headers.begin(); i != frame_headers.end(); ++i )
    {
      output_order.push_back( csv_element( i->first, i->first == world_gcs_fh, /* is_track = */ false ) );
    }
  }

  LOG_DEBUG( "Writing " << track_headers.size() << " track headers, "
             << frame_headers.size() << " frame headers; output order length "
             << output_order.size() );

  // emit the header line
  for (size_t i=0, n_fields=output_order.size(); i<n_fields; ++i)
  {
    const vector<string>& v =
      (output_order[i].is_track)
      ? track_headers[ output_order[i].fh ]
      : frame_headers[ output_order[i].fh ];
    for (size_t j=0, n_headers=v.size(); j<n_headers ; ++j)
    {
      bool last_column = ((i==n_fields-1) && (j==n_headers-1));
      string comma = (last_column) ? string("") : string(",");
      os << v[j] << comma;
    }
  }
  os << "\n";

  // emit the data

  timer.mark();
  for (size_t i=0; i<tracks.size(); ++i )
  {
    if ( timer.real() > 5 * 1000 )
    {
      ostringstream oss;
      oss << "CSV: Wrote " << i << " of " << tracks.size();
      LOG_INFO( oss.str() );
      timer.mark();
    }
    this->emit_pool_as_csv( os, output_order, tracks[i].row, i, /* is_track = */ true, csv_v1_semantics );

    frame_handle_list_type frames = this->unlocked_get_frames( tracks[i] );
    for (size_t j=0; j<frames.size(); ++j )
    {
      this->emit_pool_as_csv( os, output_order, frames[j].row, i, /* is_track = */ false, csv_v1_semantics );
    }
  }

  // all done
  return true;
}

csv_handler_map_type
track_oracle_impl
::get_csv_handler_map( const vector< string >& headers )
{
  boost::unique_lock< boost::mutex > lock( this->api_lock );


  map< string, field_handle_type > header_map;
  map< size_t, size_t > header_claimed_map;
  csv_handler_map_type m;

  // initialize the header_map
  for (size_t i=0; i<headers.size(); ++i)
  {
    header_map[ headers[i] ] = INVALID_FIELD_HANDLE;
  }

  // tag the elements in the header_map with their associated field_handle
  for ( map< field_handle_type, element_store_base* >::const_iterator i = this->element_pool.begin();
        i !=this->element_pool.end();
        ++i )
  {

    const element_descriptor& this_ed = i->second->get_descriptor();
    const string local_debug_name = "";
    bool local_debug = this_ed.name == local_debug_name;

    // skip the system headers
    if ( this_ed.role == element_descriptor::SYSTEM ) continue;


    vector< string > element_headers = i->second->csv_headers();
    if ( local_debug ) LOG_DEBUG("gchm: " << local_debug_name << " has " << element_headers.size() << " headers");
    csv_header_index_type header_indices;

    for (size_t j=0; j<element_headers.size(); ++j)
    {
      const string& this_header = element_headers[j];
      // does the caller's header map contain this element header?
      vector< string >::const_iterator p = find( headers.begin(), headers.end(), this_header );
      if ( local_debug ) LOG_DEBUG("gchm: " << local_debug_name << " ('" << this_header << "') in caller's header? " << (p != headers.end() ));

      // no: continue.
      if ( p == headers.end() ) continue;

      // yes: make sure nobody's claimed it already
      map< string, field_handle_type >::iterator check_p = header_map.find( this_header );
      if ( check_p->second != INVALID_FIELD_HANDLE )
      {
        // whoops: name the offenders
        const element_descriptor& that_ed = this->element_pool[ check_p->second ]->get_descriptor();
        throw runtime_error( "CSV header collision: header '" + element_headers[j] + "' is claimed by both "
                                 + this_ed.name + " and " + that_ed.name );
      }

      // remember the index
      size_t this_header_index = p - headers.begin();
      header_indices.push_back( this_header_index );
      // mark the header as claimed
      header_map[ this_header ] = i->first;
      ++header_claimed_map[ this_header_index ];

      if (local_debug) LOG_DEBUG("gchm: " << local_debug_name << " is at header index " << this_header_index << " and has handle " << i->first );

    } // ... for all headers used by this element

    // if header_indices.empty(), then this set of headers doesn't mention this
    // data element.  Move along.

    if ( header_indices.empty() ) continue;


    // If all has gone well, header_indices now lists indices for the
    // element's CSV headers in the caller's headers vector, in order.
    // Check that they're the same size (in case, for example, the CSV
    // only has three of four columns for a vgl_box, due to a cut-n-paste
    // error or something.)

    if ( element_headers.size() != header_indices.size() )
    {
      LOG_ERROR( "CSV header: " << this_ed.name << " requires " << element_headers.size() <<
                 " headers but only "<< header_indices.size() << " supplied" );
      {
        ostringstream oss;
        for (size_t j=0; j<element_headers.size(); ++j) oss << element_headers[j] << " ";
        LOG_ERROR( "CSV header: requires headers '" << oss.str() << "'" );
      }
      {
        ostringstream oss;
        for (size_t j=0; j<header_indices.size(); ++j) oss << headers[ header_indices[j] ] << " ";
        LOG_ERROR( "CSV header: supplied headers '" << oss.str() << "'" );
      }
      LOG_ERROR( "Ignoring supplied headers" );
    }
    else
    {
      // all is well!  Associate header_indices with the field handle.
      m[ i->first ] = header_indices;
      if (local_debug) LOG_DEBUG("gchm: " << local_debug_name << " mapping " << i->first << " to " << header_indices.size() << " headers" );
    }

  } // ...for all possible fields

  // Associate all unclaimed headers with the INVALID_FIELD_HANDLE slot.
  csv_header_index_type unclaimed_indices;

  for (map< size_t, size_t>::const_iterator i = header_claimed_map.begin();
       i != header_claimed_map.end();
       ++i)
  {
    // if == 1, used exactly once-- continue.
    if ( i->second == 1 ) continue;

    // if == 0, remember it as unclaimed and continue
    if ( i->second == 0 )
    {
      unclaimed_indices.push_back( i->first );
      continue;
    }

    // Shouldn't get here-- If > 1 , logic error-- we used the header twice!
    throw runtime_error( "CSV parser: header "+headers[i->first]+" used multiple times?" );
  }

  m[ INVALID_FIELD_HANDLE ] = unclaimed_indices;

  // all done!
  return m;

}


} // namespace vidtk

