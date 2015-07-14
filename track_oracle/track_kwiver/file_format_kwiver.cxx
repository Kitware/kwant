/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "file_format_kwiver.h"

#include <iostream>
#include <fstream>

#include <tinyxml.h>

#include <track_oracle/xml_tokenizer.h>
#include <track_oracle/element_store_base.h>
#include <track_oracle/logging_map.h>

#include <logger/logger.h>


using std::ofstream;
using std::ostream;
using std::string;
using std::vector;
using std::string;

VIDTK_LOGGER( "file_format_kwxml" );


namespace // anon
{
using namespace vidtk;


struct kwiver_xml_helper
{
  logging_map_type* w;
  void add_field_to_row( oracle_entry_handle_type row, const string& name, TiXmlNode* node );
  void read_frame( TiXmlNode* xml_f, frame_handle_type f );
  track_handle_type read_track( TiXmlNode* xml_t );
  kwiver_xml_helper()
    : w( new logging_map_type( VIDTK_DEFAULT_LOGGER, VIDTK_LOGGER_SITE ))
  {}
  ~kwiver_xml_helper()
  {
    delete w;
  }
};

void
kwiver_xml_helper
::add_field_to_row( oracle_entry_handle_type row,
                    const string& name,
                    TiXmlNode* node )
{
  TiXmlElement* e = node->ToElement();
  if ( ! e )
  {
    LOG_ERROR( "Couldn't convert '" << name << "' to an element at row " << node->Row() );
    return;
  }
  field_handle_type fh = track_oracle::lookup_by_name( name );
  if ( fh == INVALID_FIELD_HANDLE )
  {
    this->w->add_msg( "Ignoring unknown field '"+name+"'" );
    return;
  }
  element_store_base* b = track_oracle::get_mutable_element_store_base( fh );
  if (! b->read_kwiver_xml_to_row( row, e ))
  {
    this->w->add_msg( "Failed to add instance of '"+name+"'" );
    return;
  }
}

void
kwiver_xml_helper
::read_frame( TiXmlNode* xml_f,
              frame_handle_type f )
{
  TiXmlNode* n = 0;
  while ( (n = xml_f->IterateChildren( n )) )
  {
    string name( n->Value() );
    this->add_field_to_row( f.row, name, n );
  }
}

track_handle_type
kwiver_xml_helper
::read_track( TiXmlNode* xml_t )
{
  TiXmlNode* n = 0;
  track_kwiver_type trk;
  track_handle_type t = trk.create();

  while ( (n = xml_t->IterateChildren( n )) )
  {
    string name( n->Value() );
    if ( name == "frame" )
    {
      frame_handle_type f = trk( t ).create_frame();
      this->read_frame( n, f );
    }
    else
    {
      this->add_field_to_row( t.row, name, n );
    }
  }

  return t;
}

} // anon


namespace vidtk
{

bool
file_format_kwiver
::inspect_file( const string& fn ) const
{
  vector< string > tokens = xml_tokenizer::first_n_tokens( fn, 1 );
  for (size_t i=0; i<tokens.size(); ++i)
  {
    if (tokens[i].find( "<kwiver>" ) != string::npos) return true;
  }
  return false;
}

bool
file_format_kwiver
::read( const string& fn,
        track_handle_list_type& tracks ) const
{
  // dig through the XML wrappers...

  kwiver_xml_helper helper;

  LOG_INFO( "TinyXML loading '" << fn << "': start" );
  TiXmlDocument doc( fn.c_str() );
  TiXmlHandle doc_handle( &doc );
  if ( ! doc.LoadFile() )
  {
    LOG_ERROR("TinyXML (KWXML) couldn't load '" << fn << "'; skipping\n");
    return false;
  }
  LOG_INFO( "TinyXML loading '" << fn << "': complete" );

  TiXmlNode* xml_root = doc.RootElement();
  if ( ! xml_root )
  {
    LOG_ERROR("Couldn't load root element from '" << fn << "'; skipping\n");
    return false;
  }

  if ( string("kwiver") != xml_root->Value() )
  {
    LOG_ERROR( "Root node of '" << fn << "' was '" << xml_root->Value() << "'; expecting 'kwiver'; skipping\n" );
    return false;
  }

  TiXmlNode* xml_track_objects = 0;
  logging_map_type wmap( VIDTK_DEFAULT_LOGGER, VIDTK_LOGGER_SITE );

  const string track_str("track");
  while( (xml_track_objects = xml_root->IterateChildren( xml_track_objects )) )
  {
    // only interested in track nodes at the moment
    if (xml_track_objects->Value() != track_str) continue;

    track_handle_type t = helper.read_track( xml_track_objects );

    if ( t.is_valid() )
    {
      tracks.push_back( t );
    }
  }

  LOG_INFO( "Read kwiver file " << fn << ": " << helper.w->n_msgs() << " warnings" );
  helper.w->dump_msgs();

  return true;
}

bool
file_format_kwiver
::write( const string& fn,
         const track_handle_list_type& tracks ) const
{
  ofstream ofs(fn.c_str());
  return ofs && this->write(ofs, tracks);
}

bool
file_format_kwiver
::write( ostream& os,
         const track_handle_list_type& tracks ) const
{
  return track_oracle::write_kwiver( os, tracks );
}



} // vidtk
