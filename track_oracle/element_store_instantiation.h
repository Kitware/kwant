/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_ELEMENT_STORE_INSTANCES_H
#define INCL_ELEMENT_STORE_INSTANCES_H

#include <track_oracle/element_store.txx>

#define ELEMENT_STORE_INSTANCES(T) \
  template void ::kwiver::kwant::element_store<T>::set_io_handler( ::kwiver::kwant::kwiver_io_base<T>* ); \
  template ::kwiver::kwant::kwiver_io_base<T>* ::kwiver::kwant::element_store<T>::get_io_handler() const;


#endif
