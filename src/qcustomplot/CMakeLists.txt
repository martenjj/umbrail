#########################################################################
#  Project:		Track Editor					#
#  Edit:		20-Sep-21					#
#########################################################################


#########################################################################
#									#
#  Compile options for QCustomPlot only					#
#									#
#########################################################################

# Normally these warnings are useful, but until the upstream source
# has been cleaned of them they are too noisy.
#
# warning: declaration of 'name' shadows a member of 'class'
add_compile_options(-Wno-shadow)
# warning: 'virtual type class::member()' can be marked override
add_compile_options(-Wno-suggest-override)

#########################################################################
#									#
#  Building the target							#
#									#
#########################################################################

add_library(qcustomplot STATIC qcustomplot.cpp)
target_link_libraries(qcustomplot
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
)
