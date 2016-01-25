/*ckwg +5
 * Copyright 2014-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_XML_OUTPUT_COMMON_CLASSES_H
#define INCL_XML_OUTPUT_COMMON_CLASSES_H

///
/// This file declares the implementation of XML writers for common elements.
///

#include <set>
#include <track_oracle/element_store.h>
#include <track_oracle/aries_interface/aries_interface.h>
#include <track_oracle/track_scorable_mgrs/scorable_mgrs.h>
#include <vgl/vgl_box_2d.h>
#include <vital/types/timestamp.h>
#include <utilities/uuid_able.h>

namespace kwiver {
namespace kwant {

template<> inline
std::ostream&
element_store<std::string>
::emit_as_XML_typed( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  bool emitted = false;
  const element_descriptor& ed = this->get_descriptor();
  std::map<oracle_entry_handle_type, std::string >::const_iterator p = this->storage.find( h );
  if (p != this->storage.end())
  {
    if (ed.name == "time_stamp_str")
    {
      os << "<timeStamp> " << p->second << " </timeStamp>\n";
      emitted = true;
    }
    else if (ed.name == "track_style")
    {
      os << "<trackStyle> " << p->second << " </trackStyle>\n";
      emitted = true;
    }
    else if (ed.name == "clip_filename")
    {
      os << "<clipFilename> " << p->second << " </clipFilename>\n";
      emitted = true;
    }
    else if (ed.name == "basic_annotation")
    {
      os << "<basicAnnotation> " << p->second << " </basicAnnotation>\n";
      emitted = true;
    }
    else if (ed.name == "augmented_annotation")
    {
      os << "<augmentedAnnotation> " << p->second << " </augmentedAnnotation>\n";
      emitted = true;
    }
  }

  return ( ! emitted )
    ? this->default_xml_output( os, h )
    : os;
}

template<> inline
std::ostream&
element_store< unsigned >
::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  bool emitted = false;
  const element_descriptor& ed = this->get_descriptor();
  std::map<oracle_entry_handle_type, unsigned >::const_iterator p = this->storage.find( h );
  if (p != this->storage.end())
  {
    if (ed.name == "video_id" )
    {
      // videoID seems to be a string in phase 1; need to improve the parser
      os << "<videoID> " << p->second << "</videoID>\n";
      emitted = true;
    }
    else if (ed.name == "event_id" )
    {
      os << "<sourceEventID> " << p->second << " </sourceEventID>\n";
      emitted = true;
    }
    else if (ed.name == "track_source_file_id" )
    {
      // would be nice to emit the name, but should wait until we
      // reorganize the code to make the access to file_format_schema cleaner
      os << "<!-- original source file ID: " << p->second << " -->\n";
      emitted = true;
    }
    else if (ed.name == "event_type" )
    {
      std::string n = aries_interface::vpd_index_to_activity( p->second );
      if (n != "" )
      {
        size_t virat_index = aries_interface::activity_to_index( n );
        os << "<activity domain=\"VIRAT\" name=\"" << n << "\" index=\"" << virat_index
           << "\" <!-- VPD source: index " << p->second << "--> />\n";
        emitted = true;
      }
      else
      {
        os << "<!-- VPD source: index " << p->second << " but no VIRAT equivalent -->\n";
      }
    }
  }

  return ( ! emitted )
    ? this->default_xml_output( os, h )
    : os;
}

template<> inline
std::ostream&
element_store< double >
::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  bool emitted = false;
  const element_descriptor& ed = this->get_descriptor();
  std::map<oracle_entry_handle_type, double >::const_iterator p = this->storage.find( h );
  if (p != this->storage.end())
  {
    if (ed.name == "descriptor_query_result_score")
    {
      os << "<queryResultScore> " << p->second << " </queryResultScore>\n";
      emitted = true;
    }
    // 'event_probability' is a kiwverism
    else if ( (ed.name == "activity_probability") || (ed.name == "event_probability" ))
    {
      os << "<activityProbability> " << p->second << " </activityProbability>\n";
      emitted = true;
    }
    else if (ed.name == "relevancy")
    {
      os << "<relevancy> " << p->second << " </relevancy>\n";
      emitted = true;
    }
    else if (ed.name == "start_time_secs")
    {
      os << "<start_time_secs> " << p->second << " </start_time_secs>\n";
      emitted = true;
    }
    else if (ed.name == "end_time_secs")
    {
      os << "<end_time_secs> " << p->second << " </end_time_secs>\n";
      emitted = true;
    }
    else if (ed.name == "latitude")
    {
      std::streamsize old_prec = os.precision( 10 );
      os << "<latitude> " << p->second << " </latitude>\n";
      os.precision( old_prec );
      emitted = true;
    }
    else if (ed.name == "longitude")
    {
      std::streamsize old_prec = os.precision( 10 );
      os << "<longitude> " << p->second << " </longitude>\n";
      os.precision( old_prec );
      emitted = true;
    }


  }

  return ( ! emitted )
    ? this->default_xml_output( os, h )
    : os;
}


template<> inline
std::ostream&
element_store< std::vector<unsigned > >
::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  bool emitted = false;
  const element_descriptor& ed = this->get_descriptor();
  std::map<oracle_entry_handle_type, std::vector< unsigned > >::const_iterator p = this->storage.find( h );
  if (p != this->storage.end())
  {
    const std::vector<unsigned>& v = p->second;

    if (ed.name == "source_track_ids")
    {
      os << "<sourceTrackIDs> ";
      for (size_t i=0; i<v.size(); ++i)
      {
        os << v[i] << " ";
      }
      os << "</sourceTrackIDs>\n";
      emitted = true;
    }
  }

  return ( ! emitted )
    ? this->default_xml_output( os, h )
    : os;
}


template<> inline
std::ostream&
element_store< int >
::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  bool emitted = false;
  const element_descriptor& ed = this->get_descriptor();
  std::map<oracle_entry_handle_type, int >::const_iterator p = this->storage.find( h );
  if (p != this->storage.end())
  {
    const int& v = p->second;

    // 'event_type' was an unsigned in kwxml, but is an int in kwiver
    if ((ed.name == "activity") || (ed.name == "event_type"))
    {
      const std::map< size_t, std::string >& i2a = aries_interface::index_to_activity_map();
      std::map< size_t, std::string>::const_iterator probe = i2a.find( v );
      std::string a =
        (probe != i2a.end())
        ? probe->second
        : "undefined";

      os << "<activity domain=\"VIRAT\" name=\"" << a << "\" index=\"" << v << "\" />\n";
      emitted = true;
    }

  }

  return ( ! emitted )
    ? this->default_xml_output( os, h )
    : os;
}



// no standalone XML output for anything of these types yet
// (some types, such as vgl_box_2d<double>, are handled in the
// track_oracle::output_kwxml() routine; these default handlers
// are provided to make the linker happy

 template<> inline std::ostream& element_store< bool >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< track_handle_list_type >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< frame_handle_list_type >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output ( os, h ); }

 template<> inline std::ostream& element_store< unsigned long >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< unsigned long long >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< vgl_box_2d<double> >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< vgl_box_2d<unsigned> >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< vgl_point_2d<double> >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< vgl_point_3d<double> >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< std::vector< std::string > >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< std::set< std::string > >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< vital::timestamp >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< std::vector< vital::timestamp > >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< uuid_t >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< std::pair< unsigned, unsigned  > >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const { return this->default_xml_output( os, h ); }

 template<> inline std::ostream& element_store< scorable_mgrs >::emit_as_XML_typed( std::ostream& os, const oracle_entry_handle_type& h) const { return this->default_xml_output( os, h ); }

} // ...kwant
} // ...kwiver


#endif
