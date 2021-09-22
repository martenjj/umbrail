

########### install files ###############

set(pics_SRCS
  # Application actions
  application_side_list.png
  application_side_tree.png
  # Data item icons
  chart_track.png
  chart_track-32.png
  chart_segment.png
  chart_segment-32.png
  chart_point.png
  chart_point-32.png
  chart_route.png
  chart_route-32.png
  chart_routepoint.png
  chart_routepoint-32.png
  # Coloured waypoint icon images
  waypoint-16.png
  waypoint-24.png
  waypoint-32.png
)

install(FILES ${pics_SRCS} DESTINATION ${KDE_INSTALL_DATADIR}/${PN}/pics)
