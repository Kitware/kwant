/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

///
/// An example program demonstrating track introspection.
///

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <string>
#include <cstdlib>
#include <stdexcept>

#include <vul/vul_arg.h>

#include <track_oracle/track_oracle.h>
#include <track_oracle/element_descriptor.h>
#include <track_oracle/element_store_base.h>
#include <track_oracle/track_field.h>
#include <track_oracle/file_format_type.h>
#include <track_oracle/file_format_schema.h>
#include <track_oracle/file_format_manager.h>
#include <track_oracle/file_format_base.h>
#include <track_oracle/csv_tokenizer.h>
#include <track_oracle/data_terms/data_terms.h>

#include <track_oracle/state_flags.h>

#include <logger/logger.h>


using std::ifstream;
using std::map;
using std::ofstream;
using std::ostringstream;
using std::string;
using std::vector;


#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_track_reader_example_cxx__
VIDTK_LOGGER("track_reader_example_cxx");


using namespace vidtk;

int load_tracks( const string& fn, track_handle_list_type& tracks, const string& unique_key );
int probe_formats( const string& fn );

int main( int argc, char *argv[] )
{
  vul_arg< bool > probe_arg( "-probe", "probe the file with each format's inspection routine" );
  vul_arg< string > fn_arg( "-f", "track file to be loaded" );
  vul_arg< string > kwxml_arg( "-k", "write out as kwxml to this file" );
  vul_arg< string > kwiver_arg( "-kwiver", "write out as kwiver to this file" );
  vul_arg< string > kw18_arg( "-kw18", "write out as kw18 to this file" );
  vul_arg< string > csv_arg( "-csv", "write out as csv to this file" );
  vul_arg< string > csv_v1_arg( "-csv-v1", "write out as old-style csv to this file" );
  vul_arg< string > kwxml_ts_arg( "-kwxml_ts", "if writing kwxml, set default track style to this", "trackObjectKitware" );
  vul_arg< string > tag_arg( "-tag", "if writing kwiver, test tags by setting track-level 'test' flag to this" );
  vul_arg< string > unique_arg( "-u", "enumerate unique XML values of this field", "" );
  vul_arg< string > test_tracks_arg( "-t", "write test tracks to this file and exit", "");
  vul_arg< string > test_headers_arg( "-th", "test tracks will only contain these headers (plus a few others)", "" );

  vul_arg_parse( argc, argv );
  file_format_manager::initialize();

  if ( test_tracks_arg.set() )
  {
    csv_handler_map_type keep_headers;
    if ( test_headers_arg.set() )
    {
      ifstream is( test_headers_arg().c_str() );
      if ( ! is )
      {
        LOG_ERROR( "Couldn't open test headers file '" << test_headers_arg() << "'" );
        return EXIT_FAILURE;
      }
      vector< string > values;
      csv_tokenizer::get_record(is, values);
      keep_headers = track_oracle::get_csv_handler_map( values );
      LOG_INFO( "Found " << values.size() << " headers naming " << keep_headers.size() << " elements in "  << test_headers_arg() );
    }
    bool rc = file_format_manager::write_test_tracks( test_tracks_arg(), keep_headers );
    LOG_INFO( "write_test_tracks returned " << rc );
    return EXIT_SUCCESS;
  }

  if ( ! fn_arg.set() )
  {
    LOG_INFO( "Usage: " << argv[0] << " [-probe] [-k kwxml-output-file] -f trackfile\n"
             << "Attempts to load the trackfile and describe its contents.\n"
             << "-probe option probes file with each format's inspection routine.");
    return EXIT_FAILURE;
  }


  int rc;
  if (probe_arg())
  {
    rc = probe_formats( fn_arg() );
  }
  else
  {
    track_handle_list_type tracks;
    rc = load_tracks( fn_arg(), tracks, unique_arg() );
    if ( kwxml_arg.set() )
    {
      track_field< dt::tracking::track_style > kwxml_ts;
      ofstream os( kwxml_arg().c_str() );
      if ( os )
      {
        for (size_t i=0; i<tracks.size(); ++i)
        {
          if ( ! kwxml_ts.exists( tracks[i].row ))
          {
            kwxml_ts( tracks[i].row ) = kwxml_ts_arg();
          }
        }
        track_oracle::write_kwxml( os, tracks );
      }
      else
      {
        LOG_ERROR( "Couldn't open '" << kwxml_arg() << "'" );
      }
    }
    if ( kwiver_arg.set() )
    {
      ofstream os( kwiver_arg().c_str() );
      if ( os )
      {
        if ( tag_arg.set() )
        {
          track_field< dt::utility::state_flags > state_flags;
          for (size_t i=0; i<tracks.size(); ++i)
          {
            state_flags( tracks[i].row ).set_flag( "test", tag_arg() );
          }
        }
        track_oracle::write_kwiver( os, tracks );
      }
      else
      {
        LOG_ERROR( "Couldn't open '" << kwiver_arg() << "'" );
      }
    }
    if ( kw18_arg.set() )
    {
      bool okay = file_format_manager::get_format( TF_KW18 )->write( kw18_arg(), tracks );
      LOG_INFO( "Wrote kw18 to " << kw18_arg() << " success: " << okay );
    }
    if ( csv_arg.set() || csv_v1_arg.set() )
    {
      bool csv_v1_semantics = csv_v1_arg.set();
      string fn = ( csv_v1_semantics ) ? csv_v1_arg() : csv_arg();
      ofstream os( fn.c_str() );
      if ( os )
      {
        bool okay = track_oracle::write_csv( os, tracks, csv_v1_semantics );
        LOG_INFO( "Wrote CSV to " << csv_arg() << " success: " << okay << " : v1 semantics " << csv_v1_semantics );
      }
      else
      {
        LOG_ERROR( "Could not write CSV to '" << csv_arg() << "'" );
      }
    }
  }
  return rc;
}

int probe_formats( const string& track_fn )
{
  const format_map_type& format_map = file_format_manager::get_format_map();
  for ( format_map_cit i = format_map.begin(); i != format_map.end(); ++i )
  {
    LOG_INFO( file_format_type::to_string( i->second->get_format() )
             << ": glob match? " << i->second->filename_matches_globs( track_fn )
             << "; inspection: " << i->second->inspect_file( track_fn )
             << "");
  }
  return EXIT_SUCCESS;
}

string
trim( string s )
{
  size_t trim = s.find_last_not_of( " \n\r\t" );
  if ( trim == string::npos )
  {
    s.clear();
  }
  else
  {
    s.erase( trim+1 );
  }
  return s;
}

int load_tracks( const string& track_fn, track_handle_list_type& tracks, const string& unique_key )
{
  if (! file_format_manager::read( track_fn, tracks ))
  {
    LOG_ERROR( "Error: couldn't read tracks from '" << track_fn << "'; exiting");
    return EXIT_FAILURE;
  }
  if ( tracks.empty() )
  {
    LOG_INFO( "Info: reader succeeded but no tracks were loaded?  Weird!");
    return EXIT_FAILURE;
  }

  // what format are they?
  file_format_schema_type ffs;
  LOG_INFO( "Loaded " << tracks.size() << " tracks, "
           << "format: " << file_format_type::to_string( ffs( tracks[0] ).format() )
           << "");

  // what do they contain?
  map< field_handle_type, size_t > track_stats, frame_stats;
  size_t frame_count = 0;

  for (size_t i=0; i<tracks.size(); ++i)
  {
    vector< field_handle_type > ts = track_oracle::fields_at_row( tracks[i].row );
    for (size_t t=0; t<ts.size(); ++t) ++track_stats[ ts[t] ];

    frame_handle_list_type frames = track_oracle::get_frames( tracks[i] );
    frame_count += frames.size();
    for (size_t j=0; j<frames.size(); ++j)
    {
      vector< field_handle_type > fs = track_oracle::fields_at_row( frames[j].row );
      for (size_t t=0; t<fs.size(); ++t) ++frame_stats[ fs[t] ];
    }
  }

  // report out
  LOG_INFO( tracks.size() << " tracks contain...");
  for (map< field_handle_type, size_t >::const_iterator i = track_stats.begin();
       i != track_stats.end();
       ++i)
  {
    element_descriptor e = track_oracle::get_element_descriptor( i->first );
    LOG_INFO( i->second << " / " << tracks.size() << ":\t" << e.name << "");
  }

  LOG_INFO( frame_count << " frames contain...");
  for (map< field_handle_type, size_t >::const_iterator i = frame_stats.begin();
       i != frame_stats.end();
       ++i)
  {
    element_descriptor e = track_oracle::get_element_descriptor( i->first );
    LOG_INFO( i->second << " / " << frame_count << ":\t" << e.name << "");
  }

  // if requested, display unique values and counts of requested field
  if (unique_key != "")
  {
    const element_store_base* const esb = track_oracle::get_element_store_base( track_oracle::lookup_by_name( unique_key ));
    if (! esb )
    {
      LOG_ERROR( "No such field '" << unique_key << "' in the tracks");
      return EXIT_FAILURE;
    }

    map< string, size_t > counts;
    for (size_t i=0; i<tracks.size(); ++i)
    {
      if ( esb->exists( tracks[i].row ))
      {
        ostringstream oss;
        esb->emit_as_kwiver( oss, tracks[i].row, "" );
        ++counts[ trim( oss.str() ) ];
      }
      frame_handle_list_type frames = track_oracle::get_frames( tracks[i] );
      for (size_t j=0; j < frames.size(); ++j)
      {
        if ( esb->exists( frames[j].row ))
        {
          ostringstream oss;
          esb->emit_as_XML( oss, frames[j].row );
          ++counts[ trim( oss.str() ) ];
        }
      }
    }
    LOG_INFO( "Counts of '" << unique_key << "' values ( N = " << counts.size() << " ):" );
    for (map< string, size_t >::const_iterator i = counts.begin();
         i != counts.end();
         ++i)
    {
      LOG_INFO( "Count: " <<  i->second << " '" << i->first << "'" );
    }
  }

  return EXIT_SUCCESS;
}