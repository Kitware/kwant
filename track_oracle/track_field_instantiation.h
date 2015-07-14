/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_FIELD_INSTANTIATION_H
#define INCL_TRACK_FIELD_INSTANTIATION_H

#include <track_oracle/track_field.txx>
#include <track_oracle/track_field_io_proxy.txx>

#define TRACK_FIELD_INSTANCES_GENERAL(T) \
  template vidtk::track_field<T>::track_field( const vidtk::track_field<T>& );     \
  template vidtk::track_field<T>& vidtk::track_field<T>::operator=( const vidtk::track_field<T>& ); \
  template vidtk::track_field<T>::Type vidtk::track_field<T>::operator()() const; \
  template vidtk::track_field<T>::Type& vidtk::track_field<T>::operator()(); \
  template vidtk::track_field<T>::Type vidtk::track_field<T>::operator()( const vidtk::oracle_entry_handle_type& ) const; \
  template vidtk::track_field<T>::Type& vidtk::track_field<T>::operator()( const vidtk::oracle_entry_handle_type&  ); \
  template void vidtk::track_field<T>::remove_at_row( const vidtk::oracle_entry_handle_type& ); \
  template vidtk::oracle_entry_handle_type vidtk::track_field<T>::lookup( const vidtk::track_field<T>::Type&, vidtk::domain_handle_type ) ; \
  template vidtk::oracle_entry_handle_type vidtk::track_field<T>::lookup( const vidtk::track_field<T>::Type&, const vidtk::track_handle_type& ) ; \
  template bool vidtk::track_field<T>::exists( const oracle_entry_handle_type& ) const; \
  template bool vidtk::track_field<T>::exists( void ) const; \
  template std::pair< bool, vidtk::track_field<T>::Type > vidtk::track_field<T>::get( vidtk::oracle_entry_handle_type ) const; \
  template vidtk::track_field<T>* vidtk::track_field<T>::clone() const; \
  template void vidtk::track_field<T>::copy_value( const vidtk::oracle_entry_handle_type&, const vidtk::oracle_entry_handle_type& ) const; \
  template std::pair< vidtk::oracle_entry_handle_type, vidtk::track_field<T>::Type > vidtk::track_field<T>::apply_functor( vidtk::track_field_functor<T>& ) const; \
  template vidtk::field_handle_type vidtk::track_field<T>::lookup_or_create_element_store( const std::string & ); \
  template vidtk::track_field_io_proxy< vidtk::track_field<T>::Type > vidtk::track_field<T>::io() const; \
  template vidtk::track_field_io_proxy< vidtk::track_field<T>::Type > vidtk::track_field<T>::io( const vidtk::oracle_entry_handle_type& ) const; \
  template vidtk::track_field_io_proxy< vidtk::track_field<T>::Type > vidtk::track_field<T>::io_fmt( const vidtk::track_field<T>::Type& ) const; \
  template std::ostream& vidtk::operator<< <vidtk::track_field<T>::Type> ( std::ostream&, const vidtk::track_field_io_proxy<vidtk::track_field<T>::Type>& );


#define TRACK_FIELD_INSTANCES_DATA_TERM_SPECIAL_OUTPUT(T) \
  TRACK_FIELD_INSTANCES_GENERAL(T) \
  template vidtk::track_field<T>::track_field();                        \
  template vidtk::track_field<T>::track_field( vidtk::track_field_host * );

#define TRACK_FIELD_INSTANCES_OLD_STYLE_SPECIAL_OUTPUT(T) \
  TRACK_FIELD_INSTANCES_GENERAL(T) \
  template vidtk::track_field<T>::track_field( const std::string& ); \
  template vidtk::track_field<T>::track_field( const std::string&, vidtk::track_field_host* );

#define TRACK_FIELD_INSTANCES_DATA_TERM_DEFAULT_OUTPUT(T) \
  TRACK_FIELD_INSTANCES_GENERAL(T) \
  template vidtk::track_field<T>::track_field();                        \
  template vidtk::track_field<T>::track_field( vidtk::track_field_host * ); \
  template std::ostream& vidtk::operator<< < T >( std::ostream&, const vidtk::track_field< T >& );

#define TRACK_FIELD_INSTANCES_OLD_STYLE_DEFAULT_OUTPUT(T) \
  TRACK_FIELD_INSTANCES_GENERAL(T) \
  template vidtk::track_field<T>::track_field( const std::string& ); \
  template vidtk::track_field<T>::track_field( const std::string&, vidtk::track_field_host* ); \
  template std::ostream& vidtk::operator<< < T >( std::ostream&, const vidtk::track_field< T >& );

// argh, commas

#define TF_MACRO_COMMA ,

#define TRACK_FIELD_INSTANCES_OLD_STYLE_SPECIAL_OUTPUT_COMMA(T, T2)        \
  template vidtk::track_field<T, T2>::track_field( const std::string& );    \
  template vidtk::track_field<T, T2>::track_field( const std::string&, vidtk::track_field_host* ); \
  template std::ostream& vidtk::operator<< < T, T2 >( std::ostream&, const vidtk::track_field< T, T2 >& );

#undef TF_MACRO_COMMA

#define TRACK_FIELD_INSTANCES_DATA_TERM(T) \
  template vidtk::track_field<T>::track_field(); \
  template vidtk::track_field<T>::track_field( vidtk::track_field_host * ); \
  template vidtk::track_field<T>::track_field( const vidtk::track_field<T>& );     \
  template vidtk::track_field<T>& vidtk::track_field<T>::operator=( const vidtk::track_field<T>& ); \
  template vidtk::track_field<T>::Type vidtk::track_field<T>::operator()() const; \
  template vidtk::track_field<T>::Type& vidtk::track_field<T>::operator()(); \
  template vidtk::track_field<T>::Type vidtk::track_field<T>::operator()( const vidtk::oracle_entry_handle_type& ) const; \
  template vidtk::track_field<T>::Type& vidtk::track_field<T>::operator()( const vidtk::oracle_entry_handle_type&  ); \
  template void vidtk::track_field<T>::remove_at_row( const vidtk::oracle_entry_handle_type& ); \
  template vidtk::oracle_entry_handle_type vidtk::track_field<T>::lookup( const vidtk::track_field<T>::Type&, vidtk::domain_handle_type ) ; \
  template vidtk::oracle_entry_handle_type vidtk::track_field<T>::lookup( const vidtk::track_field<T>::Type&, const vidtk::track_handle_type& ) ; \
  template bool vidtk::track_field<T>::exists( const oracle_entry_handle_type& ) const; \
  template bool vidtk::track_field<T>::exists( void ) const; \
  template std::pair< bool, vidtk::track_field<T>::Type > vidtk::track_field<T>::get( vidtk::oracle_entry_handle_type ) const; \
  template vidtk::track_field<T>* vidtk::track_field<T>::clone() const; \
  template void vidtk::track_field<T>::copy_value( const vidtk::oracle_entry_handle_type&, const vidtk::oracle_entry_handle_type& ) const; \
  template std::pair< vidtk::oracle_entry_handle_type, vidtk::track_field<T>::Type > vidtk::track_field<T>::apply_functor( vidtk::track_field_functor<Type>& ) const; \
  template vidtk::track_field_io_proxy< vidtk::track_field<T>::Type > vidtk::track_field<T>::io() const; \
  template vidtk::track_field_io_proxy< vidtk::track_field<T>::Type > vidtk::track_field<T>::io( const vidtk::oracle_entry_handle_type& ) const; \
  template vidtk::track_field_io_proxy< vidtk::track_field<T>::Type > vidtk::track_field<T>::io_fmt( const vidtk::track_field<T>::Type& ) const; \
  template vidtk::field_handle_type vidtk::track_field<T>::lookup_or_create_element_store( const std::string & ); \
  template std::ostream& vidtk::operator<< ( std::ostream&, const vidtk::track_field<T>&  );


#endif
