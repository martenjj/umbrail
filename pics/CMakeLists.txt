

########### install files ###############

set(pics_SRCS
  application_side_list.png
  application_side_tree.png
  chart_track.png
  chart_segment.png
  chart_point.png
  chart_route.png
  waypoint_16.png
  waypoint_24.png
  waypoint_32.png
)

install(FILES ${pics_SRCS} DESTINATION ${KDE_INSTALL_DATADIR}/${PN}/pics)
