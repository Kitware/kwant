/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_ELEMENT_STORE_INSTANCES_H
#define INCL_ELEMENT_STORE_INSTANCES_H

#include <track_oracle/element_store.txx>

#define ELEMENT_STORE_INSTANCES(T) \
  template std::ostream& vidtk::element_store<T>::default_xml_output( std::ostream& os, const oracle_entry_handle_type& h ) const;


#endif
