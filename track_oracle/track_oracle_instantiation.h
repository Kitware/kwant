/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_ORACLE_INSTANCES_H
#define INCL_TRACK_ORACLE_INSTANCES_H

#include <track_oracle/track_oracle_impl.txx>
#include <track_oracle/track_oracle.txx>

#define TRACK_ORACLE_INSTANCES(T) \
  template vidtk::field_handle_type vidtk::track_oracle_impl::unlocked_create_element<T>( const vidtk::element_descriptor& e ); \
  template vidtk::field_handle_type vidtk::track_oracle::create_element<T>( const vidtk::element_descriptor& e ); \
  template T& vidtk::track_oracle_impl::unlocked_get_field<T>( vidtk::oracle_entry_handle_type track, vidtk::field_handle_type field ); \
  template T& vidtk::track_oracle::get_field<T>( vidtk::oracle_entry_handle_type track, vidtk::field_handle_type field ); \
  template std::pair< bool, T > vidtk::track_oracle::get<T>( const vidtk::oracle_entry_handle_type& track, const vidtk::field_handle_type& field ); \
  template std::pair< bool, T > vidtk::track_oracle_impl::get<T>( vidtk::oracle_entry_handle_type track, vidtk::field_handle_type field ); \
  template vidtk::oracle_entry_handle_type vidtk::track_oracle::lookup<T>( vidtk::field_handle_type field, const T& val, vidtk::domain_handle_type domain ); \
  template void vidtk::track_oracle::remove_field<T>( vidtk::oracle_entry_handle_type row, vidtk::field_handle_type field );\
  template std::pair< std::map<vidtk::oracle_entry_handle_type, T>*, T> vidtk::track_oracle_impl::lookup_table<T>( vidtk::field_handle_type field ); \
  template T& vidtk::track_oracle_impl::get_field<T>( vidtk::oracle_entry_handle_type track, vidtk::field_handle_type field ); \
  template void vidtk::track_oracle_impl::remove_field<T>( vidtk::oracle_entry_handle_type row, vidtk::field_handle_type field ); \
  template vidtk::oracle_entry_handle_type vidtk::track_oracle_impl::lookup<T>( vidtk::field_handle_type field, const T& val, vidtk::domain_handle_type domain );

#endif
