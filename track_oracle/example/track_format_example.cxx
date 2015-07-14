/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

///
/// An example program demonstrating track schema introspection.
///

#include <iostream>
#include <map>
#include <string>
#include <cstdlib>

#include <track_oracle/track_oracle.h>
#include <track_oracle/track_base.h>
#include <track_oracle/element_descriptor.h>
#include <track_oracle/file_format_manager.h>
#include <track_oracle/file_format_base.h>
#include <track_oracle/schema_factory.h>

#include <logger/logger.h>


using std::map;
using std::ostringstream;
using std::string;
using std::vector;


#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_track_format_example_cxx__
VIDTK_LOGGER("track_format_example_cxx");


using namespace vidtk;

// holds our on-the-fly schema
struct adhoc_schema: public track_base< adhoc_schema >
{
};

void dump_all_formats();
void match_against_schema( int argc, char *argv[] );

int main( int argc, char *argv[] )
{
  if (argc == 2)
  {
    string opt( argv[1] );
    if ((opt == "-h") || (opt == "-?"))
    {
      LOG_INFO( "Usage: " << argv[0] << " [field-name] [field-name ...]\n"
               << "With no arguments, lists all fields of known track formats\n"
               << "With field names, construct schema from fields and return track formats that match it");
      return EXIT_FAILURE;
    }
  }

  if (argc == 1)
  {
    dump_all_formats();
  }
  else
  {
    match_against_schema( argc, argv );
  }
}

void
dump_all_formats()
{
  for (size_t tfi = TF_BEGIN; tfi != TF_INVALID_TYPE; ++tfi)
  {
    file_format_enum tf = static_cast< file_format_enum >( tfi );
    file_format_base* ff = file_format_manager::get_format( tf );
    if ( ! ff )
    {
      LOG_INFO( "No format manager for enumeration " << tfi << "?");
      continue;
    }

    map< field_handle_type, track_base_impl::schema_position_type > schema_elements =
      ff->enumerate_schema();
    LOG_INFO( "Format " << file_format_type::to_string( ff->get_format() ) << " (" << ff->format_description() << ") contains "
             << schema_elements.size() << " elements:");
    for (map< field_handle_type, track_base_impl::schema_position_type >::const_iterator i = schema_elements.begin();
         i != schema_elements.end();
         ++i)
    {
      ostringstream oss;
      switch (i->second)
      {
      case track_base_impl::IN_TRACK: oss << "(track)"; break;
      case track_base_impl::IN_FRAME: oss << "(frame)"; break;
      default: oss << "(unknown?)" ; break;
      }
      element_descriptor e = track_oracle::get_element_descriptor( i->first );
      oss << "\t " << e.name;
      LOG_INFO( oss.str() );
    }
    LOG_INFO( "schema enumeration complete");
  }
}

void
match_against_schema( int argc, char *argv[] )
{
  adhoc_schema schema;

  // add fields into the schema
  for (int i = 1; i < argc; ++i)
  {
    bool rc = schema_factory::clone_field_into_schema( schema, argv[i] );
    LOG_INFO( "Cloned '" << argv[i] << "' into schema: " << rc << "");
  }

  // create a gratuitous instance of it and print
  {
    track_handle_type t = schema.create();
    /*frame_handle_type f =*/ schema.create_frame();
    LOG_INFO( "Final schema: " << schema( t ) << "");
  }

  // what matches it?
  vector< file_format_enum > matches = file_format_manager::format_matches_schema( schema );
  LOG_INFO( "Schema matches " << matches.size() << " formats:");
  for (size_t i = 0; i< matches.size(); ++i)
  {
    LOG_INFO( file_format_type::to_string( matches[i] ) << "");
  }

}
