
set( vidtk_stanag_4676_sources
  Acceleration.h                        Acceleration.cxx
  Area.h                                Area.cxx
  CircularArea.h                        CircularArea.cxx
  CovarianceMatrix.h                    CovarianceMatrix.cxx
  CovarianceMatrixPosition.h            CovarianceMatrixPosition.cxx
  CovarianceMatrixPositionVelocity.h    CovarianceMatrixPositionVelocity.cxx
  DataSource.h                          DataSource.cxx
  Enums.h                               Enums.cxx
  Exceptions.h                          Exceptions.cxx
  GeodeticPosition.h                    GeodeticPosition.cxx
  IDdata.h                              IDdata.cxx
  LocalCartesianAcceleration.h          LocalCartesianAcceleration.cxx
  LocalCartesianPosition.h              LocalCartesianPosition.cxx
  LocalCartesianVelocity.h              LocalCartesianVelocity.cxx
  LocalCoordinateSystem.h               LocalCoordinateSystem.cxx
  Macros.h
  PixelPosition.h                       PixelPosition.cxx
  PolygonArea.h                         PolygonArea.cxx
  Position.h                            Position.cxx
  Security.h                            Security.cxx
  Track                                 Track.cxx
  TrackItem.h                           TrackItem.cxx
  TrackMessage.h                        TrackMessage.cxx
  TrackPoint.h                          TrackPoint.cxx
  TrackPointDetail.h                    TrackPointDetail.cxx
  TrackProductType.h                    TrackProductType.cxx
  TrackSource.h                         TrackSource.cxx
  XMLIOBase.h                           XMLIOBase.cxx
  Velocity.h                            Velocity.cxx

  stanag_4676_fwd.h                     stanag_4676.h
)

add_library( vidtk_stanag_4676 ${vidtk_stanag_4676_sources} )

set_target_properties( vidtk_stanag_4676 PROPERTIES
  VERSION ${vidtk_VERSION} SOVERSION ${vidtk_VERSION}
)

target_link_libraries( vidtk_stanag_4676
  vidtk_logger
  ${TinyXML_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
)

install( TARGETS vidtk_stanag_4676
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
