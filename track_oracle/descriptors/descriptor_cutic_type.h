/*ckwg +5
 * Copyright 2011-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_DESCRIPTOR_CUTIC_H
#define INCL_DESCRIPTOR_CUTIC_H

#include <vector>
#include <iostream>


typedef struct descriptor_cutic_type
{
  //object level classifiers..
  //      (size=number of active models)
  std::vector< double >           score_class;
  std::vector< int >              score_type;

  //temporal continuity
  //      (size= usually 3, within interval, between last interval, overall track)
  std::vector< double >           sim_temporal;

  //raw descriptors (exclusively used in distance computation)
  //      (size=non-zero BOW), index=vector index, raw=value for that word index
  //              *NOTE* assumption of unique,increasing index values in vector
  std::vector< short >            desc_index;
  std::vector< double >           desc_raw;

  bool operator==(const descriptor_cutic_type& a) const
  {
    return (this->score_class == a.score_class) &&
      (this->score_type == a.score_type) &&
      (this->sim_temporal == a.sim_temporal) &&
      (this->desc_index == a.desc_index) &&
      (this->desc_raw == a.desc_raw);
  }

} descriptor_cutic_type;

std::ostream& operator<<( std::ostream& os, const descriptor_cutic_type& );
std::istream& operator>>( std::istream& is, descriptor_cutic_type& );

#endif /* INCL_DESCRIPTOR_CUTIC_H */
