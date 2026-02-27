#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_3525274203873677771) {
   out_3525274203873677771[0] = delta_x[0] + nom_x[0];
   out_3525274203873677771[1] = delta_x[1] + nom_x[1];
   out_3525274203873677771[2] = delta_x[2] + nom_x[2];
   out_3525274203873677771[3] = delta_x[3] + nom_x[3];
   out_3525274203873677771[4] = delta_x[4] + nom_x[4];
   out_3525274203873677771[5] = delta_x[5] + nom_x[5];
   out_3525274203873677771[6] = delta_x[6] + nom_x[6];
   out_3525274203873677771[7] = delta_x[7] + nom_x[7];
   out_3525274203873677771[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_297560133990373422) {
   out_297560133990373422[0] = -nom_x[0] + true_x[0];
   out_297560133990373422[1] = -nom_x[1] + true_x[1];
   out_297560133990373422[2] = -nom_x[2] + true_x[2];
   out_297560133990373422[3] = -nom_x[3] + true_x[3];
   out_297560133990373422[4] = -nom_x[4] + true_x[4];
   out_297560133990373422[5] = -nom_x[5] + true_x[5];
   out_297560133990373422[6] = -nom_x[6] + true_x[6];
   out_297560133990373422[7] = -nom_x[7] + true_x[7];
   out_297560133990373422[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_3681855867780802793) {
   out_3681855867780802793[0] = 1.0;
   out_3681855867780802793[1] = 0.0;
   out_3681855867780802793[2] = 0.0;
   out_3681855867780802793[3] = 0.0;
   out_3681855867780802793[4] = 0.0;
   out_3681855867780802793[5] = 0.0;
   out_3681855867780802793[6] = 0.0;
   out_3681855867780802793[7] = 0.0;
   out_3681855867780802793[8] = 0.0;
   out_3681855867780802793[9] = 0.0;
   out_3681855867780802793[10] = 1.0;
   out_3681855867780802793[11] = 0.0;
   out_3681855867780802793[12] = 0.0;
   out_3681855867780802793[13] = 0.0;
   out_3681855867780802793[14] = 0.0;
   out_3681855867780802793[15] = 0.0;
   out_3681855867780802793[16] = 0.0;
   out_3681855867780802793[17] = 0.0;
   out_3681855867780802793[18] = 0.0;
   out_3681855867780802793[19] = 0.0;
   out_3681855867780802793[20] = 1.0;
   out_3681855867780802793[21] = 0.0;
   out_3681855867780802793[22] = 0.0;
   out_3681855867780802793[23] = 0.0;
   out_3681855867780802793[24] = 0.0;
   out_3681855867780802793[25] = 0.0;
   out_3681855867780802793[26] = 0.0;
   out_3681855867780802793[27] = 0.0;
   out_3681855867780802793[28] = 0.0;
   out_3681855867780802793[29] = 0.0;
   out_3681855867780802793[30] = 1.0;
   out_3681855867780802793[31] = 0.0;
   out_3681855867780802793[32] = 0.0;
   out_3681855867780802793[33] = 0.0;
   out_3681855867780802793[34] = 0.0;
   out_3681855867780802793[35] = 0.0;
   out_3681855867780802793[36] = 0.0;
   out_3681855867780802793[37] = 0.0;
   out_3681855867780802793[38] = 0.0;
   out_3681855867780802793[39] = 0.0;
   out_3681855867780802793[40] = 1.0;
   out_3681855867780802793[41] = 0.0;
   out_3681855867780802793[42] = 0.0;
   out_3681855867780802793[43] = 0.0;
   out_3681855867780802793[44] = 0.0;
   out_3681855867780802793[45] = 0.0;
   out_3681855867780802793[46] = 0.0;
   out_3681855867780802793[47] = 0.0;
   out_3681855867780802793[48] = 0.0;
   out_3681855867780802793[49] = 0.0;
   out_3681855867780802793[50] = 1.0;
   out_3681855867780802793[51] = 0.0;
   out_3681855867780802793[52] = 0.0;
   out_3681855867780802793[53] = 0.0;
   out_3681855867780802793[54] = 0.0;
   out_3681855867780802793[55] = 0.0;
   out_3681855867780802793[56] = 0.0;
   out_3681855867780802793[57] = 0.0;
   out_3681855867780802793[58] = 0.0;
   out_3681855867780802793[59] = 0.0;
   out_3681855867780802793[60] = 1.0;
   out_3681855867780802793[61] = 0.0;
   out_3681855867780802793[62] = 0.0;
   out_3681855867780802793[63] = 0.0;
   out_3681855867780802793[64] = 0.0;
   out_3681855867780802793[65] = 0.0;
   out_3681855867780802793[66] = 0.0;
   out_3681855867780802793[67] = 0.0;
   out_3681855867780802793[68] = 0.0;
   out_3681855867780802793[69] = 0.0;
   out_3681855867780802793[70] = 1.0;
   out_3681855867780802793[71] = 0.0;
   out_3681855867780802793[72] = 0.0;
   out_3681855867780802793[73] = 0.0;
   out_3681855867780802793[74] = 0.0;
   out_3681855867780802793[75] = 0.0;
   out_3681855867780802793[76] = 0.0;
   out_3681855867780802793[77] = 0.0;
   out_3681855867780802793[78] = 0.0;
   out_3681855867780802793[79] = 0.0;
   out_3681855867780802793[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_1710696929836261316) {
   out_1710696929836261316[0] = state[0];
   out_1710696929836261316[1] = state[1];
   out_1710696929836261316[2] = state[2];
   out_1710696929836261316[3] = state[3];
   out_1710696929836261316[4] = state[4];
   out_1710696929836261316[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_1710696929836261316[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_1710696929836261316[7] = state[7];
   out_1710696929836261316[8] = state[8];
}
void F_fun(double *state, double dt, double *out_1799025913951617520) {
   out_1799025913951617520[0] = 1;
   out_1799025913951617520[1] = 0;
   out_1799025913951617520[2] = 0;
   out_1799025913951617520[3] = 0;
   out_1799025913951617520[4] = 0;
   out_1799025913951617520[5] = 0;
   out_1799025913951617520[6] = 0;
   out_1799025913951617520[7] = 0;
   out_1799025913951617520[8] = 0;
   out_1799025913951617520[9] = 0;
   out_1799025913951617520[10] = 1;
   out_1799025913951617520[11] = 0;
   out_1799025913951617520[12] = 0;
   out_1799025913951617520[13] = 0;
   out_1799025913951617520[14] = 0;
   out_1799025913951617520[15] = 0;
   out_1799025913951617520[16] = 0;
   out_1799025913951617520[17] = 0;
   out_1799025913951617520[18] = 0;
   out_1799025913951617520[19] = 0;
   out_1799025913951617520[20] = 1;
   out_1799025913951617520[21] = 0;
   out_1799025913951617520[22] = 0;
   out_1799025913951617520[23] = 0;
   out_1799025913951617520[24] = 0;
   out_1799025913951617520[25] = 0;
   out_1799025913951617520[26] = 0;
   out_1799025913951617520[27] = 0;
   out_1799025913951617520[28] = 0;
   out_1799025913951617520[29] = 0;
   out_1799025913951617520[30] = 1;
   out_1799025913951617520[31] = 0;
   out_1799025913951617520[32] = 0;
   out_1799025913951617520[33] = 0;
   out_1799025913951617520[34] = 0;
   out_1799025913951617520[35] = 0;
   out_1799025913951617520[36] = 0;
   out_1799025913951617520[37] = 0;
   out_1799025913951617520[38] = 0;
   out_1799025913951617520[39] = 0;
   out_1799025913951617520[40] = 1;
   out_1799025913951617520[41] = 0;
   out_1799025913951617520[42] = 0;
   out_1799025913951617520[43] = 0;
   out_1799025913951617520[44] = 0;
   out_1799025913951617520[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_1799025913951617520[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_1799025913951617520[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_1799025913951617520[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_1799025913951617520[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_1799025913951617520[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_1799025913951617520[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_1799025913951617520[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_1799025913951617520[53] = -9.8100000000000005*dt;
   out_1799025913951617520[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_1799025913951617520[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_1799025913951617520[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_1799025913951617520[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_1799025913951617520[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_1799025913951617520[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_1799025913951617520[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_1799025913951617520[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_1799025913951617520[62] = 0;
   out_1799025913951617520[63] = 0;
   out_1799025913951617520[64] = 0;
   out_1799025913951617520[65] = 0;
   out_1799025913951617520[66] = 0;
   out_1799025913951617520[67] = 0;
   out_1799025913951617520[68] = 0;
   out_1799025913951617520[69] = 0;
   out_1799025913951617520[70] = 1;
   out_1799025913951617520[71] = 0;
   out_1799025913951617520[72] = 0;
   out_1799025913951617520[73] = 0;
   out_1799025913951617520[74] = 0;
   out_1799025913951617520[75] = 0;
   out_1799025913951617520[76] = 0;
   out_1799025913951617520[77] = 0;
   out_1799025913951617520[78] = 0;
   out_1799025913951617520[79] = 0;
   out_1799025913951617520[80] = 1;
}
void h_25(double *state, double *unused, double *out_5416149830362286688) {
   out_5416149830362286688[0] = state[6];
}
void H_25(double *state, double *unused, double *out_1045298942765376599) {
   out_1045298942765376599[0] = 0;
   out_1045298942765376599[1] = 0;
   out_1045298942765376599[2] = 0;
   out_1045298942765376599[3] = 0;
   out_1045298942765376599[4] = 0;
   out_1045298942765376599[5] = 0;
   out_1045298942765376599[6] = 1;
   out_1045298942765376599[7] = 0;
   out_1045298942765376599[8] = 0;
}
void h_24(double *state, double *unused, double *out_7084306918088164219) {
   out_7084306918088164219[0] = state[4];
   out_7084306918088164219[1] = state[5];
}
void H_24(double *state, double *unused, double *out_8188902388611608138) {
   out_8188902388611608138[0] = 0;
   out_8188902388611608138[1] = 0;
   out_8188902388611608138[2] = 0;
   out_8188902388611608138[3] = 0;
   out_8188902388611608138[4] = 1;
   out_8188902388611608138[5] = 0;
   out_8188902388611608138[6] = 0;
   out_8188902388611608138[7] = 0;
   out_8188902388611608138[8] = 0;
   out_8188902388611608138[9] = 0;
   out_8188902388611608138[10] = 0;
   out_8188902388611608138[11] = 0;
   out_8188902388611608138[12] = 0;
   out_8188902388611608138[13] = 0;
   out_8188902388611608138[14] = 1;
   out_8188902388611608138[15] = 0;
   out_8188902388611608138[16] = 0;
   out_8188902388611608138[17] = 0;
}
void h_30(double *state, double *unused, double *out_5691343892646792577) {
   out_5691343892646792577[0] = state[4];
}
void H_30(double *state, double *unused, double *out_3482397387362231599) {
   out_3482397387362231599[0] = 0;
   out_3482397387362231599[1] = 0;
   out_3482397387362231599[2] = 0;
   out_3482397387362231599[3] = 0;
   out_3482397387362231599[4] = 1;
   out_3482397387362231599[5] = 0;
   out_3482397387362231599[6] = 0;
   out_3482397387362231599[7] = 0;
   out_3482397387362231599[8] = 0;
}
void h_26(double *state, double *unused, double *out_7270316149721926584) {
   out_7270316149721926584[0] = state[7];
}
void H_26(double *state, double *unused, double *out_2696204376108679625) {
   out_2696204376108679625[0] = 0;
   out_2696204376108679625[1] = 0;
   out_2696204376108679625[2] = 0;
   out_2696204376108679625[3] = 0;
   out_2696204376108679625[4] = 0;
   out_2696204376108679625[5] = 0;
   out_2696204376108679625[6] = 0;
   out_2696204376108679625[7] = 1;
   out_2696204376108679625[8] = 0;
}
void h_27(double *state, double *unused, double *out_341835083631451268) {
   out_341835083631451268[0] = state[3];
}
void H_27(double *state, double *unused, double *out_1258803316178288382) {
   out_1258803316178288382[0] = 0;
   out_1258803316178288382[1] = 0;
   out_1258803316178288382[2] = 0;
   out_1258803316178288382[3] = 1;
   out_1258803316178288382[4] = 0;
   out_1258803316178288382[5] = 0;
   out_1258803316178288382[6] = 0;
   out_1258803316178288382[7] = 0;
   out_1258803316178288382[8] = 0;
}
void h_29(double *state, double *unused, double *out_7112670309981802204) {
   out_7112670309981802204[0] = state[1];
}
void H_29(double *state, double *unused, double *out_2972166043047839415) {
   out_2972166043047839415[0] = 0;
   out_2972166043047839415[1] = 1;
   out_2972166043047839415[2] = 0;
   out_2972166043047839415[3] = 0;
   out_2972166043047839415[4] = 0;
   out_2972166043047839415[5] = 0;
   out_2972166043047839415[6] = 0;
   out_2972166043047839415[7] = 0;
   out_2972166043047839415[8] = 0;
}
void h_28(double *state, double *unused, double *out_2530122726332441526) {
   out_2530122726332441526[0] = state[0];
}
void H_28(double *state, double *unused, double *out_8054565060117369989) {
   out_8054565060117369989[0] = 1;
   out_8054565060117369989[1] = 0;
   out_8054565060117369989[2] = 0;
   out_8054565060117369989[3] = 0;
   out_8054565060117369989[4] = 0;
   out_8054565060117369989[5] = 0;
   out_8054565060117369989[6] = 0;
   out_8054565060117369989[7] = 0;
   out_8054565060117369989[8] = 0;
}
void h_31(double *state, double *unused, double *out_2419756905528375528) {
   out_2419756905528375528[0] = state[8];
}
void H_31(double *state, double *unused, double *out_1075944904642337027) {
   out_1075944904642337027[0] = 0;
   out_1075944904642337027[1] = 0;
   out_1075944904642337027[2] = 0;
   out_1075944904642337027[3] = 0;
   out_1075944904642337027[4] = 0;
   out_1075944904642337027[5] = 0;
   out_1075944904642337027[6] = 0;
   out_1075944904642337027[7] = 0;
   out_1075944904642337027[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_3525274203873677771) {
  err_fun(nom_x, delta_x, out_3525274203873677771);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_297560133990373422) {
  inv_err_fun(nom_x, true_x, out_297560133990373422);
}
void car_H_mod_fun(double *state, double *out_3681855867780802793) {
  H_mod_fun(state, out_3681855867780802793);
}
void car_f_fun(double *state, double dt, double *out_1710696929836261316) {
  f_fun(state,  dt, out_1710696929836261316);
}
void car_F_fun(double *state, double dt, double *out_1799025913951617520) {
  F_fun(state,  dt, out_1799025913951617520);
}
void car_h_25(double *state, double *unused, double *out_5416149830362286688) {
  h_25(state, unused, out_5416149830362286688);
}
void car_H_25(double *state, double *unused, double *out_1045298942765376599) {
  H_25(state, unused, out_1045298942765376599);
}
void car_h_24(double *state, double *unused, double *out_7084306918088164219) {
  h_24(state, unused, out_7084306918088164219);
}
void car_H_24(double *state, double *unused, double *out_8188902388611608138) {
  H_24(state, unused, out_8188902388611608138);
}
void car_h_30(double *state, double *unused, double *out_5691343892646792577) {
  h_30(state, unused, out_5691343892646792577);
}
void car_H_30(double *state, double *unused, double *out_3482397387362231599) {
  H_30(state, unused, out_3482397387362231599);
}
void car_h_26(double *state, double *unused, double *out_7270316149721926584) {
  h_26(state, unused, out_7270316149721926584);
}
void car_H_26(double *state, double *unused, double *out_2696204376108679625) {
  H_26(state, unused, out_2696204376108679625);
}
void car_h_27(double *state, double *unused, double *out_341835083631451268) {
  h_27(state, unused, out_341835083631451268);
}
void car_H_27(double *state, double *unused, double *out_1258803316178288382) {
  H_27(state, unused, out_1258803316178288382);
}
void car_h_29(double *state, double *unused, double *out_7112670309981802204) {
  h_29(state, unused, out_7112670309981802204);
}
void car_H_29(double *state, double *unused, double *out_2972166043047839415) {
  H_29(state, unused, out_2972166043047839415);
}
void car_h_28(double *state, double *unused, double *out_2530122726332441526) {
  h_28(state, unused, out_2530122726332441526);
}
void car_H_28(double *state, double *unused, double *out_8054565060117369989) {
  H_28(state, unused, out_8054565060117369989);
}
void car_h_31(double *state, double *unused, double *out_2419756905528375528) {
  h_31(state, unused, out_2419756905528375528);
}
void car_H_31(double *state, double *unused, double *out_1075944904642337027) {
  H_31(state, unused, out_1075944904642337027);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
