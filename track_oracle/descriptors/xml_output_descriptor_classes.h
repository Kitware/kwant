/*ckwg +5
 * Copyright 2014-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_XML_OUTPUT_DESCRIPTOR_CLASSES_H
#define INCL_XML_OUTPUT_DESCRIPTOR_CLASSES_H

///
/// This file holds the declarations of XML writers for the descriptors.
///

#include <track_oracle/element_store.h>
#include <track_oracle/descriptors/descriptor_cutic_type.h>
#include <track_oracle/descriptors/descriptor_event_label_type.h>
#include <track_oracle/descriptors/descriptor_metadata_type.h>
#include <track_oracle/descriptors/descriptor_motion_type.h>
#include <track_oracle/descriptors/descriptor_overlap_type.h>
#include <track_oracle/descriptors/descriptor_raw_1d_type.h>
#include <track_oracle/aries_interface/aries_interface.h>

namespace kwiver {
namespace kwant {

template<> inline std::ostream& element_store< std::vector< double > >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  bool emitted = false;
  const element_descriptor& ed = this->get_descriptor();
  std::map<oracle_entry_handle_type, std::vector< double > >::const_iterator p = this->storage.find( h );
  if ( p != this->storage.end())
  {
    const std::vector<double>& v = p->second;
    if (ed.name == "descriptor_classifier")
    {
      const std::map< size_t, std::string >& m = aries_interface::index_to_activity_map();
      os << "<descriptor type=\"classifier\">\n";
      for (size_t i=0; i<v.size(); ++i)
      {
        if (v[i] != 0)
        {
          std::map< size_t, std::string >::const_iterator probe = m.find( i );
          if ( probe == m.end() )
          {
            os << "<!-- invalid activity index " << i << " w/ probability " << v[i] << " -->\n";
          }
          else
          {
            os << "<probability activity=\"" << probe->second << "\" value=\"" << v[i] << "\" />\n";
          }
        }
      }
      os << "</descriptor>\n";
      emitted = true;
    }
  }
  return ( ! emitted )
    ? this->default_xml_output( os, h )
    : os;
}


template<> inline std::ostream& element_store< std::vector< std::vector< double > > >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const {  return this->default_xml_output( os, h ); }

template<> inline std::ostream& element_store< descriptor_cutic_type >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const {  return this->default_xml_output( os, h ); }

template<> inline std::ostream& element_store< descriptor_metadata_type >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const {  return this->default_xml_output( os, h ); }

template<> inline std::ostream& element_store< descriptor_motion_type >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const {  return this->default_xml_output( os, h ); }

template<> inline std::ostream& element_store< descriptor_overlap_type >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  std::map<oracle_entry_handle_type, descriptor_overlap_type >::const_iterator p = this->storage.find( h );
  os << p->second;
  return os;
}

template<> inline std::ostream& element_store< descriptor_raw_1d_type >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  std::map<oracle_entry_handle_type, descriptor_raw_1d_type >::const_iterator p = this->storage.find( h );
  os << p->second;
  return os;
}

template<> inline std::ostream& element_store< descriptor_event_label_type >::emit_as_XML( std::ostream& os, const oracle_entry_handle_type& h ) const
{
  std::map<oracle_entry_handle_type, descriptor_event_label_type >::const_iterator p = this->storage.find( h );
  os << p->second;
  return os;
}

} // ...kwant
} // ...kwiver

#endif
