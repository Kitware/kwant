/*ckwg +5
 * Copyright 2014-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_ORACLE_INSTANCES_H
#define INCL_TRACK_ORACLE_INSTANCES_H

#include <track_oracle/track_oracle_impl.txx>
#include <track_oracle/track_oracle.txx>

#define TRACK_ORACLE_INSTANCES(T) \
  template kwiver::kwant::field_handle_type kwiver::kwant::track_oracle_impl::unlocked_create_element<T>( const kwiver::kwant::element_descriptor& e ); \
  template kwiver::kwant::field_handle_type kwiver::kwant::track_oracle::create_element<T>( const kwiver::kwant::element_descriptor& e ); \
  template T& kwiver::kwant::track_oracle_impl::unlocked_get_field<T>( kwiver::kwant::oracle_entry_handle_type track, kwiver::kwant::field_handle_type field ); \
  template T& kwiver::kwant::track_oracle::get_field<T>( kwiver::kwant::oracle_entry_handle_type track, kwiver::kwant::field_handle_type field ); \
  template std::pair< bool, T > kwiver::kwant::track_oracle::get<T>( const kwiver::kwant::oracle_entry_handle_type& track, const kwiver::kwant::field_handle_type& field ); \
  template std::pair< bool, T > kwiver::kwant::track_oracle_impl::get<T>( kwiver::kwant::oracle_entry_handle_type track, kwiver::kwant::field_handle_type field ); \
  template kwiver::kwant::oracle_entry_handle_type kwiver::kwant::track_oracle::lookup<T>( kwiver::kwant::field_handle_type field, const T& val, kwiver::kwant::domain_handle_type domain ); \
  template void kwiver::kwant::track_oracle::remove_field<T>( kwiver::kwant::oracle_entry_handle_type row, kwiver::kwant::field_handle_type field );\
  template std::pair< std::map<kwiver::kwant::oracle_entry_handle_type, T>*, T> kwiver::kwant::track_oracle_impl::lookup_table<T>( kwiver::kwant::field_handle_type field ); \
  template T& kwiver::kwant::track_oracle_impl::get_field<T>( kwiver::kwant::oracle_entry_handle_type track, kwiver::kwant::field_handle_type field ); \
  template void kwiver::kwant::track_oracle_impl::remove_field<T>( kwiver::kwant::oracle_entry_handle_type row, kwiver::kwant::field_handle_type field ); \
  template kwiver::kwant::oracle_entry_handle_type kwiver::kwant::track_oracle_impl::lookup<T>( kwiver::kwant::field_handle_type field, const T& val, kwiver::kwant::domain_handle_type domain );

#endif
