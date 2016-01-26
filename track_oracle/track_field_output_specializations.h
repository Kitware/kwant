/*ckwg +5
 * Copyright 2012-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_FIELD_OUTPUT_SPECIALIZATIONS_H
#define INCL_TRACK_FIELD_OUTPUT_SPECIALIZATIONS_H

#include <vital/vital_config.h>
#include <track_oracle/track_oracle_export.h>

#include <set>
#include <utility>
#include <track_oracle/track_oracle_api_types.h>
#include <track_oracle/track_field.h>
#include <track_oracle/descriptors/descriptor_cutic_type.h>
#include <track_oracle/descriptors/descriptor_metadata_type.h>
#include <track_oracle/descriptors/descriptor_motion_type.h>
#include <track_oracle/descriptors/descriptor_overlap_type.h>
#include <track_oracle/descriptors/descriptor_event_label_type.h>
#include <track_oracle/descriptors/descriptor_raw_1d_type.h>

namespace kwiver {
namespace kwant {

// specialization for e.g. frame lists
template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< frame_handle_list_type >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< track_handle_list_type >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< std::vector< unsigned int> >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< std::pair<unsigned int, unsigned int> >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< descriptor_cutic_type >& f);

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< descriptor_metadata_type >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< descriptor_motion_type >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< descriptor_overlap_type >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< descriptor_event_label_type >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< descriptor_raw_1d_type >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< std::vector< double> >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< std::vector< std::vector<double> > >& f);

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< std::vector< std::string> >& f );

template< >
std::ostream& TRACK_ORACLE_EXPORT
operator<<( std::ostream& os,
            const track_field< std::set< std::string> >& f );

} // ...kwant
} // ...kwiver

#endif
