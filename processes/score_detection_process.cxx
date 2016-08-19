/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
[ * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Implementation of score_detection_process
 */

#include "score_detection_process.h"

#include "kwant_type_traits.h"

#include <vital/types/timestamp.h>
#include <vital/vital_foreach.h>

#include <arrows/vxl/bounding_box.h>

#include <scoring_framework/score_core.h>
#include <scoring_framework/score_phase1.h>
#include <scoring_framework/phase1_parameters.h>

#include <memory>

namespace kwiver {
namespace kwant {

namespace kwto = ::kwiver::track_oracle;

// ----------------------------------------------------------------
/**
 * \class template_process
 *
 * \brief Process template
 *
 * \iports
 *
 * \iport{ground_truth_set} Ground truth object set
 *
 * \iport{detected_object_set} Set of detected objects.
 *
 * \oports
 *
 * \oport{image} Resulting image
 *
 * \configs
 *
 * \config{header} Header text. (string)
 *
 * \config{footer} Footer text (string)
 */

// config items
// <name>, <type>, <default string>, <description string>
create_config_trait( min_bound_matching_area, double, "0",
                     "on a single frame, area must be > min_bound_matching_area to match" );

create_config_trait( radial_overlap, double, "-1",
                     "-1 (or anything <0) to disable and use bounding boxes.  Otherwise, "
                     "distance in meters between two detections for them to match." );

create_config_trait( point_detection_box_size, unsigned, "0",
                     " when using radial overlap, to test for inclusion in an AOI, convert point-only "
                     "detections into a square box this many pixels on a side, centered on the detection. "
                     "Not used when radial_overlap < 0." );

create_config_trait( pass_all_nonzero_overlaps, bool, "false",
                     "if true, all non-zero overlaps will passed downstream for "
                     "computing e.g. purity and continuity.  If false, only those "
                     "frames passing the frame filters (min_bound_matching_area, "
                     "min_matching_frames, min_pcent_overlap_gt_ct) are passed "
                     "downstream.  Invalid if radial overlap requested." );

// There may be others

//----------------------------------------------------------------
// Private implementation class
class score_detection_process::priv
{
public:
  priv();
  ~priv();

  // Convert detection_set to kwant tracks
  kwto::track_handle_list_type convert_detections( kwiver::vital::detected_object_set_sptr set );

  void score_detections( kwiver::vital::detected_object_set_sptr ground_truth,
                    kwiver::vital::detected_object_set_sptr detections );

  // Configuration values
  phase1_parameters m_scoring_parameters;

  std::unique_ptr< kwiver::kwant::track2track_phase1 >  m_p1;

}; // end priv class

// ================================================================
score_detection_process
::score_detection_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new score_detection_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) );
  make_ports(); // create process ports
  make_config(); // declare process configuration
}


score_detection_process
::~score_detection_process()
{
}


// ----------------------------------------------------------------
/**
 * @brief Configure process
 *
 * This method is called prior to connecting ports to allow the
 * process to configure itself.
 */
void
score_detection_process
::_configure()
{
  d->m_scoring_parameters.min_bound_matching_area   = config_value_using_trait( min_bound_matching_area );
  d->m_scoring_parameters.radial_overlap            = config_value_using_trait( radial_overlap );
  d->m_scoring_parameters.point_detection_box_size  = config_value_using_trait( point_detection_box_size );
  d->m_scoring_parameters.pass_all_nonzero_overlaps = config_value_using_trait( pass_all_nonzero_overlaps );

  d->m_p1.reset( new kwiver::kwant::track2track_phase1( d->m_scoring_parameters ) );
}


// ----------------------------------------------------------------
void
score_detection_process
::_step()
{
  kwiver::vital::timestamp frame_time;

  // See if optional input port has been connected.
  // Get input only if connected.
  if ( has_input_port_edge_using_trait( timestamp ) )
  {
    frame_time = grab_from_port_using_trait( timestamp );
  }

  kwiver::vital::detected_object_set_sptr ground_truth = grab_from_port_using_trait( ground_truth_set );
  kwiver::vital::detected_object_set_sptr detections = grab_from_port_using_trait( detected_object_set );

  LOG_DEBUG( logger(), "Processing frame " << frame_time );

  //+ auto overlap_set = d->score_detections( ground_truth, detections );

  //+ push_to_port_using_trait( image, out_image );
}


// ----------------------------------------------------------------
void
score_detection_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( timestamp, optional );
  declare_input_port_using_trait( ground_truth_set, required );
  declare_input_port_using_trait( detected_object_set, required );

  // -- output --
  //+ TBD - managed overlap records
  declare_output_port_using_trait( image, optional );
}


// ----------------------------------------------------------------
void
score_detection_process
::make_config()
{
  declare_config_using_trait( min_bound_matching_area );
  declare_config_using_trait( radial_overlap );
  declare_config_using_trait( point_detection_box_size );
  declare_config_using_trait( pass_all_nonzero_overlaps );
}


// ================================================================
score_detection_process::priv
::priv()
{
}


score_detection_process::priv
::~priv()
{
}


// ------------------------------------------------------------------
kwto::track_handle_list_type
score_detection_process::priv
::convert_detections( kwiver::vital::detected_object_set_sptr set )
{
  auto detections = set->select(); // get ordered list of detections
  kwto::track_handle_list_type kwant_tracks;
  unsigned track_id(0);

  VITAL_FOREACH( auto det, detections )
  {
    scorable_track_type t;
    kwto::track_handle_type h = t.create();
    t.external_id() = track_id++;

    vgl_box_2d<double> vbox = kwiver::arrows::vxl::convert( det->bounding_box() );
    t.bounding_box() = vbox;
    t.timestamp_frame() = 1;
    t.timestamp_usecs() = 0;

    kwant_tracks.push_back(h);
  } // end foreach

  return kwant_tracks;
}


// ------------------------------------------------------------------
void
score_detection_process::priv
::score_detections( kwiver::vital::detected_object_set_sptr ground_truth,
                    kwiver::vital::detected_object_set_sptr detections )
{
  auto gt_handle = convert_detections( ground_truth );
  auto det_handle = convert_detections( detections );

  m_p1->compute_all( gt_handle, det_handle);


}


} } // end namespace
