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
void err_fun(double *nom_x, double *delta_x, double *out_411609516403737386) {
   out_411609516403737386[0] = delta_x[0] + nom_x[0];
   out_411609516403737386[1] = delta_x[1] + nom_x[1];
   out_411609516403737386[2] = delta_x[2] + nom_x[2];
   out_411609516403737386[3] = delta_x[3] + nom_x[3];
   out_411609516403737386[4] = delta_x[4] + nom_x[4];
   out_411609516403737386[5] = delta_x[5] + nom_x[5];
   out_411609516403737386[6] = delta_x[6] + nom_x[6];
   out_411609516403737386[7] = delta_x[7] + nom_x[7];
   out_411609516403737386[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_7082078735183717431) {
   out_7082078735183717431[0] = -nom_x[0] + true_x[0];
   out_7082078735183717431[1] = -nom_x[1] + true_x[1];
   out_7082078735183717431[2] = -nom_x[2] + true_x[2];
   out_7082078735183717431[3] = -nom_x[3] + true_x[3];
   out_7082078735183717431[4] = -nom_x[4] + true_x[4];
   out_7082078735183717431[5] = -nom_x[5] + true_x[5];
   out_7082078735183717431[6] = -nom_x[6] + true_x[6];
   out_7082078735183717431[7] = -nom_x[7] + true_x[7];
   out_7082078735183717431[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_1390740467130861487) {
   out_1390740467130861487[0] = 1.0;
   out_1390740467130861487[1] = 0.0;
   out_1390740467130861487[2] = 0.0;
   out_1390740467130861487[3] = 0.0;
   out_1390740467130861487[4] = 0.0;
   out_1390740467130861487[5] = 0.0;
   out_1390740467130861487[6] = 0.0;
   out_1390740467130861487[7] = 0.0;
   out_1390740467130861487[8] = 0.0;
   out_1390740467130861487[9] = 0.0;
   out_1390740467130861487[10] = 1.0;
   out_1390740467130861487[11] = 0.0;
   out_1390740467130861487[12] = 0.0;
   out_1390740467130861487[13] = 0.0;
   out_1390740467130861487[14] = 0.0;
   out_1390740467130861487[15] = 0.0;
   out_1390740467130861487[16] = 0.0;
   out_1390740467130861487[17] = 0.0;
   out_1390740467130861487[18] = 0.0;
   out_1390740467130861487[19] = 0.0;
   out_1390740467130861487[20] = 1.0;
   out_1390740467130861487[21] = 0.0;
   out_1390740467130861487[22] = 0.0;
   out_1390740467130861487[23] = 0.0;
   out_1390740467130861487[24] = 0.0;
   out_1390740467130861487[25] = 0.0;
   out_1390740467130861487[26] = 0.0;
   out_1390740467130861487[27] = 0.0;
   out_1390740467130861487[28] = 0.0;
   out_1390740467130861487[29] = 0.0;
   out_1390740467130861487[30] = 1.0;
   out_1390740467130861487[31] = 0.0;
   out_1390740467130861487[32] = 0.0;
   out_1390740467130861487[33] = 0.0;
   out_1390740467130861487[34] = 0.0;
   out_1390740467130861487[35] = 0.0;
   out_1390740467130861487[36] = 0.0;
   out_1390740467130861487[37] = 0.0;
   out_1390740467130861487[38] = 0.0;
   out_1390740467130861487[39] = 0.0;
   out_1390740467130861487[40] = 1.0;
   out_1390740467130861487[41] = 0.0;
   out_1390740467130861487[42] = 0.0;
   out_1390740467130861487[43] = 0.0;
   out_1390740467130861487[44] = 0.0;
   out_1390740467130861487[45] = 0.0;
   out_1390740467130861487[46] = 0.0;
   out_1390740467130861487[47] = 0.0;
   out_1390740467130861487[48] = 0.0;
   out_1390740467130861487[49] = 0.0;
   out_1390740467130861487[50] = 1.0;
   out_1390740467130861487[51] = 0.0;
   out_1390740467130861487[52] = 0.0;
   out_1390740467130861487[53] = 0.0;
   out_1390740467130861487[54] = 0.0;
   out_1390740467130861487[55] = 0.0;
   out_1390740467130861487[56] = 0.0;
   out_1390740467130861487[57] = 0.0;
   out_1390740467130861487[58] = 0.0;
   out_1390740467130861487[59] = 0.0;
   out_1390740467130861487[60] = 1.0;
   out_1390740467130861487[61] = 0.0;
   out_1390740467130861487[62] = 0.0;
   out_1390740467130861487[63] = 0.0;
   out_1390740467130861487[64] = 0.0;
   out_1390740467130861487[65] = 0.0;
   out_1390740467130861487[66] = 0.0;
   out_1390740467130861487[67] = 0.0;
   out_1390740467130861487[68] = 0.0;
   out_1390740467130861487[69] = 0.0;
   out_1390740467130861487[70] = 1.0;
   out_1390740467130861487[71] = 0.0;
   out_1390740467130861487[72] = 0.0;
   out_1390740467130861487[73] = 0.0;
   out_1390740467130861487[74] = 0.0;
   out_1390740467130861487[75] = 0.0;
   out_1390740467130861487[76] = 0.0;
   out_1390740467130861487[77] = 0.0;
   out_1390740467130861487[78] = 0.0;
   out_1390740467130861487[79] = 0.0;
   out_1390740467130861487[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_2246396216540142177) {
   out_2246396216540142177[0] = state[0];
   out_2246396216540142177[1] = state[1];
   out_2246396216540142177[2] = state[2];
   out_2246396216540142177[3] = state[3];
   out_2246396216540142177[4] = state[4];
   out_2246396216540142177[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_2246396216540142177[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_2246396216540142177[7] = state[7];
   out_2246396216540142177[8] = state[8];
}
void F_fun(double *state, double dt, double *out_3263326103915417388) {
   out_3263326103915417388[0] = 1;
   out_3263326103915417388[1] = 0;
   out_3263326103915417388[2] = 0;
   out_3263326103915417388[3] = 0;
   out_3263326103915417388[4] = 0;
   out_3263326103915417388[5] = 0;
   out_3263326103915417388[6] = 0;
   out_3263326103915417388[7] = 0;
   out_3263326103915417388[8] = 0;
   out_3263326103915417388[9] = 0;
   out_3263326103915417388[10] = 1;
   out_3263326103915417388[11] = 0;
   out_3263326103915417388[12] = 0;
   out_3263326103915417388[13] = 0;
   out_3263326103915417388[14] = 0;
   out_3263326103915417388[15] = 0;
   out_3263326103915417388[16] = 0;
   out_3263326103915417388[17] = 0;
   out_3263326103915417388[18] = 0;
   out_3263326103915417388[19] = 0;
   out_3263326103915417388[20] = 1;
   out_3263326103915417388[21] = 0;
   out_3263326103915417388[22] = 0;
   out_3263326103915417388[23] = 0;
   out_3263326103915417388[24] = 0;
   out_3263326103915417388[25] = 0;
   out_3263326103915417388[26] = 0;
   out_3263326103915417388[27] = 0;
   out_3263326103915417388[28] = 0;
   out_3263326103915417388[29] = 0;
   out_3263326103915417388[30] = 1;
   out_3263326103915417388[31] = 0;
   out_3263326103915417388[32] = 0;
   out_3263326103915417388[33] = 0;
   out_3263326103915417388[34] = 0;
   out_3263326103915417388[35] = 0;
   out_3263326103915417388[36] = 0;
   out_3263326103915417388[37] = 0;
   out_3263326103915417388[38] = 0;
   out_3263326103915417388[39] = 0;
   out_3263326103915417388[40] = 1;
   out_3263326103915417388[41] = 0;
   out_3263326103915417388[42] = 0;
   out_3263326103915417388[43] = 0;
   out_3263326103915417388[44] = 0;
   out_3263326103915417388[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_3263326103915417388[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_3263326103915417388[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_3263326103915417388[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_3263326103915417388[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_3263326103915417388[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_3263326103915417388[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_3263326103915417388[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_3263326103915417388[53] = -9.8100000000000005*dt;
   out_3263326103915417388[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_3263326103915417388[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_3263326103915417388[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3263326103915417388[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3263326103915417388[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_3263326103915417388[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_3263326103915417388[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_3263326103915417388[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3263326103915417388[62] = 0;
   out_3263326103915417388[63] = 0;
   out_3263326103915417388[64] = 0;
   out_3263326103915417388[65] = 0;
   out_3263326103915417388[66] = 0;
   out_3263326103915417388[67] = 0;
   out_3263326103915417388[68] = 0;
   out_3263326103915417388[69] = 0;
   out_3263326103915417388[70] = 1;
   out_3263326103915417388[71] = 0;
   out_3263326103915417388[72] = 0;
   out_3263326103915417388[73] = 0;
   out_3263326103915417388[74] = 0;
   out_3263326103915417388[75] = 0;
   out_3263326103915417388[76] = 0;
   out_3263326103915417388[77] = 0;
   out_3263326103915417388[78] = 0;
   out_3263326103915417388[79] = 0;
   out_3263326103915417388[80] = 1;
}
void h_25(double *state, double *unused, double *out_7774150008233015132) {
   out_7774150008233015132[0] = state[6];
}
void H_25(double *state, double *unused, double *out_1594416823404739290) {
   out_1594416823404739290[0] = 0;
   out_1594416823404739290[1] = 0;
   out_1594416823404739290[2] = 0;
   out_1594416823404739290[3] = 0;
   out_1594416823404739290[4] = 0;
   out_1594416823404739290[5] = 0;
   out_1594416823404739290[6] = 1;
   out_1594416823404739290[7] = 0;
   out_1594416823404739290[8] = 0;
}
void h_24(double *state, double *unused, double *out_3517564190390228050) {
   out_3517564190390228050[0] = state[4];
   out_3517564190390228050[1] = state[5];
}
void H_24(double *state, double *unused, double *out_5823974810272187603) {
   out_5823974810272187603[0] = 0;
   out_5823974810272187603[1] = 0;
   out_5823974810272187603[2] = 0;
   out_5823974810272187603[3] = 0;
   out_5823974810272187603[4] = 1;
   out_5823974810272187603[5] = 0;
   out_5823974810272187603[6] = 0;
   out_5823974810272187603[7] = 0;
   out_5823974810272187603[8] = 0;
   out_5823974810272187603[9] = 0;
   out_5823974810272187603[10] = 0;
   out_5823974810272187603[11] = 0;
   out_5823974810272187603[12] = 0;
   out_5823974810272187603[13] = 0;
   out_5823974810272187603[14] = 1;
   out_5823974810272187603[15] = 0;
   out_5823974810272187603[16] = 0;
   out_5823974810272187603[17] = 0;
}
void h_30(double *state, double *unused, double *out_3901758839126185548) {
   out_3901758839126185548[0] = state[4];
}
void H_30(double *state, double *unused, double *out_923916135102509337) {
   out_923916135102509337[0] = 0;
   out_923916135102509337[1] = 0;
   out_923916135102509337[2] = 0;
   out_923916135102509337[3] = 0;
   out_923916135102509337[4] = 1;
   out_923916135102509337[5] = 0;
   out_923916135102509337[6] = 0;
   out_923916135102509337[7] = 0;
   out_923916135102509337[8] = 0;
}
void h_26(double *state, double *unused, double *out_4894550625139811592) {
   out_4894550625139811592[0] = state[7];
}
void H_26(double *state, double *unused, double *out_5335920142278795514) {
   out_5335920142278795514[0] = 0;
   out_5335920142278795514[1] = 0;
   out_5335920142278795514[2] = 0;
   out_5335920142278795514[3] = 0;
   out_5335920142278795514[4] = 0;
   out_5335920142278795514[5] = 0;
   out_5335920142278795514[6] = 0;
   out_5335920142278795514[7] = 1;
   out_5335920142278795514[8] = 0;
}
void h_27(double *state, double *unused, double *out_8717659861134677567) {
   out_8717659861134677567[0] = state[3];
}
void H_27(double *state, double *unused, double *out_1250847176697915574) {
   out_1250847176697915574[0] = 0;
   out_1250847176697915574[1] = 0;
   out_1250847176697915574[2] = 0;
   out_1250847176697915574[3] = 1;
   out_1250847176697915574[4] = 0;
   out_1250847176697915574[5] = 0;
   out_1250847176697915574[6] = 0;
   out_1250847176697915574[7] = 0;
   out_1250847176697915574[8] = 0;
}
void h_29(double *state, double *unused, double *out_1089206735856332165) {
   out_1089206735856332165[0] = state[1];
}
void H_29(double *state, double *unused, double *out_1434147479416901521) {
   out_1434147479416901521[0] = 0;
   out_1434147479416901521[1] = 1;
   out_1434147479416901521[2] = 0;
   out_1434147479416901521[3] = 0;
   out_1434147479416901521[4] = 0;
   out_1434147479416901521[5] = 0;
   out_1434147479416901521[6] = 0;
   out_1434147479416901521[7] = 0;
   out_1434147479416901521[8] = 0;
}
void h_28(double *state, double *unused, double *out_3715986593473831922) {
   out_3715986593473831922[0] = state[0];
}
void H_28(double *state, double *unused, double *out_3648251537652629053) {
   out_3648251537652629053[0] = 1;
   out_3648251537652629053[1] = 0;
   out_3648251537652629053[2] = 0;
   out_3648251537652629053[3] = 0;
   out_3648251537652629053[4] = 0;
   out_3648251537652629053[5] = 0;
   out_3648251537652629053[6] = 0;
   out_3648251537652629053[7] = 0;
   out_3648251537652629053[8] = 0;
}
void h_31(double *state, double *unused, double *out_10884751412342691) {
   out_10884751412342691[0] = state[8];
}
void H_31(double *state, double *unused, double *out_5962128244512146990) {
   out_5962128244512146990[0] = 0;
   out_5962128244512146990[1] = 0;
   out_5962128244512146990[2] = 0;
   out_5962128244512146990[3] = 0;
   out_5962128244512146990[4] = 0;
   out_5962128244512146990[5] = 0;
   out_5962128244512146990[6] = 0;
   out_5962128244512146990[7] = 0;
   out_5962128244512146990[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_411609516403737386) {
  err_fun(nom_x, delta_x, out_411609516403737386);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_7082078735183717431) {
  inv_err_fun(nom_x, true_x, out_7082078735183717431);
}
void car_H_mod_fun(double *state, double *out_1390740467130861487) {
  H_mod_fun(state, out_1390740467130861487);
}
void car_f_fun(double *state, double dt, double *out_2246396216540142177) {
  f_fun(state,  dt, out_2246396216540142177);
}
void car_F_fun(double *state, double dt, double *out_3263326103915417388) {
  F_fun(state,  dt, out_3263326103915417388);
}
void car_h_25(double *state, double *unused, double *out_7774150008233015132) {
  h_25(state, unused, out_7774150008233015132);
}
void car_H_25(double *state, double *unused, double *out_1594416823404739290) {
  H_25(state, unused, out_1594416823404739290);
}
void car_h_24(double *state, double *unused, double *out_3517564190390228050) {
  h_24(state, unused, out_3517564190390228050);
}
void car_H_24(double *state, double *unused, double *out_5823974810272187603) {
  H_24(state, unused, out_5823974810272187603);
}
void car_h_30(double *state, double *unused, double *out_3901758839126185548) {
  h_30(state, unused, out_3901758839126185548);
}
void car_H_30(double *state, double *unused, double *out_923916135102509337) {
  H_30(state, unused, out_923916135102509337);
}
void car_h_26(double *state, double *unused, double *out_4894550625139811592) {
  h_26(state, unused, out_4894550625139811592);
}
void car_H_26(double *state, double *unused, double *out_5335920142278795514) {
  H_26(state, unused, out_5335920142278795514);
}
void car_h_27(double *state, double *unused, double *out_8717659861134677567) {
  h_27(state, unused, out_8717659861134677567);
}
void car_H_27(double *state, double *unused, double *out_1250847176697915574) {
  H_27(state, unused, out_1250847176697915574);
}
void car_h_29(double *state, double *unused, double *out_1089206735856332165) {
  h_29(state, unused, out_1089206735856332165);
}
void car_H_29(double *state, double *unused, double *out_1434147479416901521) {
  H_29(state, unused, out_1434147479416901521);
}
void car_h_28(double *state, double *unused, double *out_3715986593473831922) {
  h_28(state, unused, out_3715986593473831922);
}
void car_H_28(double *state, double *unused, double *out_3648251537652629053) {
  H_28(state, unused, out_3648251537652629053);
}
void car_h_31(double *state, double *unused, double *out_10884751412342691) {
  h_31(state, unused, out_10884751412342691);
}
void car_H_31(double *state, double *unused, double *out_5962128244512146990) {
  H_31(state, unused, out_5962128244512146990);
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
