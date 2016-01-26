/*ckwg +5
 * Copyright 2014-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_FIELD_INSTANTIATION_H
#define INCL_TRACK_FIELD_INSTANTIATION_H

#include <track_oracle/track_field.txx>
#include <track_oracle/track_field_io_proxy.txx>

#define TRACK_FIELD_INSTANCES_GENERAL(T) \
  template kwiver::kwant::track_field<T>::track_field( const kwiver::kwant::track_field<T>& );     \
  template kwiver::kwant::track_field<T>& kwiver::kwant::track_field<T>::operator=( const kwiver::kwant::track_field<T>& ); \
  template kwiver::kwant::track_field<T>::Type kwiver::kwant::track_field<T>::operator()() const; \
  template kwiver::kwant::track_field<T>::Type& kwiver::kwant::track_field<T>::operator()(); \
  template kwiver::kwant::track_field<T>::Type kwiver::kwant::track_field<T>::operator()( const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template kwiver::kwant::track_field<T>::Type& kwiver::kwant::track_field<T>::operator()( const kwiver::kwant::oracle_entry_handle_type&  ); \
  template void kwiver::kwant::track_field<T>::remove_at_row( const kwiver::kwant::oracle_entry_handle_type& ); \
  template kwiver::kwant::oracle_entry_handle_type kwiver::kwant::track_field<T>::lookup( const kwiver::kwant::track_field<T>::Type&, kwiver::kwant::domain_handle_type ) ; \
  template kwiver::kwant::oracle_entry_handle_type kwiver::kwant::track_field<T>::lookup( const kwiver::kwant::track_field<T>::Type&, const kwiver::kwant::track_handle_type& ) ; \
  template bool kwiver::kwant::track_field<T>::exists( const oracle_entry_handle_type& ) const; \
  template bool kwiver::kwant::track_field<T>::exists( void ) const; \
  template std::pair< bool, kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::get( kwiver::kwant::oracle_entry_handle_type ) const; \
  template kwiver::kwant::track_field<T>* kwiver::kwant::track_field<T>::clone() const; \
  template void kwiver::kwant::track_field<T>::copy_value( const kwiver::kwant::oracle_entry_handle_type&, const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template std::pair< kwiver::kwant::oracle_entry_handle_type, kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::apply_functor( kwiver::kwant::track_field_functor<T>& ) const; \
  template kwiver::kwant::field_handle_type kwiver::kwant::track_field<T>::lookup_or_create_element_store( const std::string & ); \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io() const; \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io( const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io_fmt( const kwiver::kwant::track_field<T>::Type& ) const; \
  template std::ostream& kwiver::kwant::operator<< <kwiver::kwant::track_field<T>::Type> ( std::ostream&, const kwiver::kwant::track_field_io_proxy<kwiver::kwant::track_field<T>::Type>& );

#define TRACK_FIELD_INSTANCES_GENERAL_DEBUG(T) \
  template kwiver::kwant::track_field<T>::track_field( const kwiver::kwant::track_field<T>& );     \
  template kwiver::kwant::track_field<T>& kwiver::kwant::track_field<T>::operator=( const kwiver::kwant::track_field<T>& ); \
  template kwiver::kwant::track_field<T>::Type kwiver::kwant::track_field<T>::operator()() const; \
  template kwiver::kwant::track_field<T>::Type& kwiver::kwant::track_field<T>::operator()(); \
  template kwiver::kwant::track_field<T>::Type kwiver::kwant::track_field<T>::operator()( const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template kwiver::kwant::track_field<T>::Type& kwiver::kwant::track_field<T>::operator()( const kwiver::kwant::oracle_entry_handle_type&  ); \
  template void kwiver::kwant::track_field<T>::remove_at_row( const kwiver::kwant::oracle_entry_handle_type& ); \
  template kwiver::kwant::oracle_entry_handle_type kwiver::kwant::track_field<T>::lookup( const kwiver::kwant::track_field<T>::Type&, kwiver::kwant::domain_handle_type ) ; \
  template kwiver::kwant::oracle_entry_handle_type kwiver::kwant::track_field<T>::lookup( const kwiver::kwant::track_field<T>::Type&, const kwiver::kwant::track_handle_type& ) ; \
  template bool kwiver::kwant::track_field<T>::exists( const oracle_entry_handle_type& ) const; \
  template bool kwiver::kwant::track_field<T>::exists( void ) const; \
  template std::pair< bool, kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::get( kwiver::kwant::oracle_entry_handle_type ) const; \
  template kwiver::kwant::track_field<T>* kwiver::kwant::track_field<T>::clone() const; \
  template void kwiver::kwant::track_field<T>::copy_value( const kwiver::kwant::oracle_entry_handle_type&, const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template std::pair< kwiver::kwant::oracle_entry_handle_type, kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::apply_functor( kwiver::kwant::track_field_functor<T>& ) const; \
  template kwiver::kwant::field_handle_type kwiver::kwant::track_field<T>::lookup_or_create_element_store( const std::string & ); \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io() const; \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io( const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io_fmt( const kwiver::kwant::track_field<T>::Type& ) const;

//  template std::ostream& kwiver::kwant::operator<< <kwiver::kwant::track_field<T>::Type> ( std::ostream&, const kwiver::kwant::track_field_io_proxy<kwiver::kwant::track_field<T>::Type>& );


#define TRACK_FIELD_INSTANCES_DATA_TERM_SPECIAL_OUTPUT(T) \
  TRACK_FIELD_INSTANCES_GENERAL(T) \
  template kwiver::kwant::track_field<T>::track_field();                        \
  template kwiver::kwant::track_field<T>::track_field( kwiver::kwant::track_field_host * );

#define TRACK_FIELD_INSTANCES_OLD_STYLE_SPECIAL_OUTPUT(T) \
  TRACK_FIELD_INSTANCES_GENERAL(T) \
  template kwiver::kwant::track_field<T>::track_field( const std::string& ); \
  template kwiver::kwant::track_field<T>::track_field( const std::string&, kwiver::kwant::track_field_host* );

#define TRACK_FIELD_INSTANCES_DATA_TERM_DEFAULT_OUTPUT(T) \
  TRACK_FIELD_INSTANCES_GENERAL(T) \
  template kwiver::kwant::track_field<T>::track_field();                        \
  template kwiver::kwant::track_field<T>::track_field( kwiver::kwant::track_field_host * ); \
  template std::ostream& kwiver::kwant::operator<< < T >( std::ostream&, const kwiver::kwant::track_field< T >& );

#define TRACK_FIELD_INSTANCES_OLD_STYLE_DEFAULT_OUTPUT(T) \
  TRACK_FIELD_INSTANCES_GENERAL(T) \
  template kwiver::kwant::track_field<T>::track_field( const std::string& ); \
  template kwiver::kwant::track_field<T>::track_field( const std::string&, kwiver::kwant::track_field_host* ); \
  template std::ostream& kwiver::kwant::operator<< < T >( std::ostream&, const kwiver::kwant::track_field< T >& );

#define TRACK_FIELD_INSTANCES_OLD_STYLE_DEFAULT_OUTPUT_DEBUG(T) \
  TRACK_FIELD_INSTANCES_GENERAL_DEBUG(T) \
  template kwiver::kwant::track_field<T>::track_field( const std::string& ); \
  template kwiver::kwant::track_field<T>::track_field( const std::string&, kwiver::kwant::track_field_host* ); \
  template std::ostream& kwiver::kwant::operator<< < T >( std::ostream&, const kwiver::kwant::track_field< T >& );

// argh, commas

#define TF_MACRO_COMMA ,

#define TRACK_FIELD_INSTANCES_OLD_STYLE_SPECIAL_OUTPUT_COMMA(T, T2)        \
  template kwiver::kwant::track_field<T, T2>::track_field( const std::string& );    \
  template kwiver::kwant::track_field<T, T2>::track_field( const std::string&, kwiver::kwant::track_field_host* ); \
  template std::ostream& kwiver::kwant::operator<< < T, T2 >( std::ostream&, const kwiver::kwant::track_field< T, T2 >& );

#undef TF_MACRO_COMMA

#define TRACK_FIELD_INSTANCES_DATA_TERM(T) \
  template kwiver::kwant::track_field<T>::track_field(); \
  template kwiver::kwant::track_field<T>::track_field( kwiver::kwant::track_field_host * ); \
  template kwiver::kwant::track_field<T>::track_field( const kwiver::kwant::track_field<T>& );     \
  template kwiver::kwant::track_field<T>& kwiver::kwant::track_field<T>::operator=( const kwiver::kwant::track_field<T>& ); \
  template kwiver::kwant::track_field<T>::Type kwiver::kwant::track_field<T>::operator()() const; \
  template kwiver::kwant::track_field<T>::Type& kwiver::kwant::track_field<T>::operator()(); \
  template kwiver::kwant::track_field<T>::Type kwiver::kwant::track_field<T>::operator()( const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template kwiver::kwant::track_field<T>::Type& kwiver::kwant::track_field<T>::operator()( const kwiver::kwant::oracle_entry_handle_type&  ); \
  template void kwiver::kwant::track_field<T>::remove_at_row( const kwiver::kwant::oracle_entry_handle_type& ); \
  template kwiver::kwant::oracle_entry_handle_type kwiver::kwant::track_field<T>::lookup( const kwiver::kwant::track_field<T>::Type&, kwiver::kwant::domain_handle_type ) ; \
  template kwiver::kwant::oracle_entry_handle_type kwiver::kwant::track_field<T>::lookup( const kwiver::kwant::track_field<T>::Type&, const kwiver::kwant::track_handle_type& ) ; \
  template bool kwiver::kwant::track_field<T>::exists( const oracle_entry_handle_type& ) const; \
  template bool kwiver::kwant::track_field<T>::exists( void ) const; \
  template std::pair< bool, kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::get( kwiver::kwant::oracle_entry_handle_type ) const; \
  template kwiver::kwant::track_field<T>* kwiver::kwant::track_field<T>::clone() const; \
  template void kwiver::kwant::track_field<T>::copy_value( const kwiver::kwant::oracle_entry_handle_type&, const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template std::pair< kwiver::kwant::oracle_entry_handle_type, kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::apply_functor( kwiver::kwant::track_field_functor<Type>& ) const; \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io() const; \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io( const kwiver::kwant::oracle_entry_handle_type& ) const; \
  template kwiver::kwant::track_field_io_proxy< kwiver::kwant::track_field<T>::Type > kwiver::kwant::track_field<T>::io_fmt( const kwiver::kwant::track_field<T>::Type& ) const; \
  template kwiver::kwant::field_handle_type kwiver::kwant::track_field<T>::lookup_or_create_element_store( const std::string & ); \
  template std::ostream& kwiver::kwant::operator<< ( std::ostream&, const kwiver::kwant::track_field<T>&  );


#endif
