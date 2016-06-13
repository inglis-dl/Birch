PROJECT( BirchCommon )

SET( BirchCommon_SRCS
  Common.cxx
  Utilities.cxx
)

ADD_LIBRARY( BirchCommon ${BirchCommon_SRCS} )

INSTALL( TARGETS BirchCommon
  DESTINATION lib 
)
