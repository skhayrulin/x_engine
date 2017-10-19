#ifndef OW_CL_STRUCT
#define OW_CL_STRUCT

#include "ow_cl_const.h"

struct extendet_particle {
  size_t p_id;
  int neigbour_list[NEIGHBOUR_COUNT];
};

#endif