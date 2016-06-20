
project(navtracks)

########### install files ###############

set(navtracks_PICS
  application_side_list.png
  application_side_tree.png
  chart_track.png
  chart_segment.png
  chart_point.png
  chart_route.png
)

install(FILES ${navtracks_PICS} DESTINATION ${DATA_INSTALL_DIR}/navtracks/pics)
