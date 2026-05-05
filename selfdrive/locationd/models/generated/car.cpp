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
void err_fun(double *nom_x, double *delta_x, double *out_4611641411917091701) {
   out_4611641411917091701[0] = delta_x[0] + nom_x[0];
   out_4611641411917091701[1] = delta_x[1] + nom_x[1];
   out_4611641411917091701[2] = delta_x[2] + nom_x[2];
   out_4611641411917091701[3] = delta_x[3] + nom_x[3];
   out_4611641411917091701[4] = delta_x[4] + nom_x[4];
   out_4611641411917091701[5] = delta_x[5] + nom_x[5];
   out_4611641411917091701[6] = delta_x[6] + nom_x[6];
   out_4611641411917091701[7] = delta_x[7] + nom_x[7];
   out_4611641411917091701[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_3968532722392907058) {
   out_3968532722392907058[0] = -nom_x[0] + true_x[0];
   out_3968532722392907058[1] = -nom_x[1] + true_x[1];
   out_3968532722392907058[2] = -nom_x[2] + true_x[2];
   out_3968532722392907058[3] = -nom_x[3] + true_x[3];
   out_3968532722392907058[4] = -nom_x[4] + true_x[4];
   out_3968532722392907058[5] = -nom_x[5] + true_x[5];
   out_3968532722392907058[6] = -nom_x[6] + true_x[6];
   out_3968532722392907058[7] = -nom_x[7] + true_x[7];
   out_3968532722392907058[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_1232909156344202498) {
   out_1232909156344202498[0] = 1.0;
   out_1232909156344202498[1] = 0.0;
   out_1232909156344202498[2] = 0.0;
   out_1232909156344202498[3] = 0.0;
   out_1232909156344202498[4] = 0.0;
   out_1232909156344202498[5] = 0.0;
   out_1232909156344202498[6] = 0.0;
   out_1232909156344202498[7] = 0.0;
   out_1232909156344202498[8] = 0.0;
   out_1232909156344202498[9] = 0.0;
   out_1232909156344202498[10] = 1.0;
   out_1232909156344202498[11] = 0.0;
   out_1232909156344202498[12] = 0.0;
   out_1232909156344202498[13] = 0.0;
   out_1232909156344202498[14] = 0.0;
   out_1232909156344202498[15] = 0.0;
   out_1232909156344202498[16] = 0.0;
   out_1232909156344202498[17] = 0.0;
   out_1232909156344202498[18] = 0.0;
   out_1232909156344202498[19] = 0.0;
   out_1232909156344202498[20] = 1.0;
   out_1232909156344202498[21] = 0.0;
   out_1232909156344202498[22] = 0.0;
   out_1232909156344202498[23] = 0.0;
   out_1232909156344202498[24] = 0.0;
   out_1232909156344202498[25] = 0.0;
   out_1232909156344202498[26] = 0.0;
   out_1232909156344202498[27] = 0.0;
   out_1232909156344202498[28] = 0.0;
   out_1232909156344202498[29] = 0.0;
   out_1232909156344202498[30] = 1.0;
   out_1232909156344202498[31] = 0.0;
   out_1232909156344202498[32] = 0.0;
   out_1232909156344202498[33] = 0.0;
   out_1232909156344202498[34] = 0.0;
   out_1232909156344202498[35] = 0.0;
   out_1232909156344202498[36] = 0.0;
   out_1232909156344202498[37] = 0.0;
   out_1232909156344202498[38] = 0.0;
   out_1232909156344202498[39] = 0.0;
   out_1232909156344202498[40] = 1.0;
   out_1232909156344202498[41] = 0.0;
   out_1232909156344202498[42] = 0.0;
   out_1232909156344202498[43] = 0.0;
   out_1232909156344202498[44] = 0.0;
   out_1232909156344202498[45] = 0.0;
   out_1232909156344202498[46] = 0.0;
   out_1232909156344202498[47] = 0.0;
   out_1232909156344202498[48] = 0.0;
   out_1232909156344202498[49] = 0.0;
   out_1232909156344202498[50] = 1.0;
   out_1232909156344202498[51] = 0.0;
   out_1232909156344202498[52] = 0.0;
   out_1232909156344202498[53] = 0.0;
   out_1232909156344202498[54] = 0.0;
   out_1232909156344202498[55] = 0.0;
   out_1232909156344202498[56] = 0.0;
   out_1232909156344202498[57] = 0.0;
   out_1232909156344202498[58] = 0.0;
   out_1232909156344202498[59] = 0.0;
   out_1232909156344202498[60] = 1.0;
   out_1232909156344202498[61] = 0.0;
   out_1232909156344202498[62] = 0.0;
   out_1232909156344202498[63] = 0.0;
   out_1232909156344202498[64] = 0.0;
   out_1232909156344202498[65] = 0.0;
   out_1232909156344202498[66] = 0.0;
   out_1232909156344202498[67] = 0.0;
   out_1232909156344202498[68] = 0.0;
   out_1232909156344202498[69] = 0.0;
   out_1232909156344202498[70] = 1.0;
   out_1232909156344202498[71] = 0.0;
   out_1232909156344202498[72] = 0.0;
   out_1232909156344202498[73] = 0.0;
   out_1232909156344202498[74] = 0.0;
   out_1232909156344202498[75] = 0.0;
   out_1232909156344202498[76] = 0.0;
   out_1232909156344202498[77] = 0.0;
   out_1232909156344202498[78] = 0.0;
   out_1232909156344202498[79] = 0.0;
   out_1232909156344202498[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_241923337051056220) {
   out_241923337051056220[0] = state[0];
   out_241923337051056220[1] = state[1];
   out_241923337051056220[2] = state[2];
   out_241923337051056220[3] = state[3];
   out_241923337051056220[4] = state[4];
   out_241923337051056220[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_241923337051056220[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_241923337051056220[7] = state[7];
   out_241923337051056220[8] = state[8];
}
void F_fun(double *state, double dt, double *out_2984795699500747305) {
   out_2984795699500747305[0] = 1;
   out_2984795699500747305[1] = 0;
   out_2984795699500747305[2] = 0;
   out_2984795699500747305[3] = 0;
   out_2984795699500747305[4] = 0;
   out_2984795699500747305[5] = 0;
   out_2984795699500747305[6] = 0;
   out_2984795699500747305[7] = 0;
   out_2984795699500747305[8] = 0;
   out_2984795699500747305[9] = 0;
   out_2984795699500747305[10] = 1;
   out_2984795699500747305[11] = 0;
   out_2984795699500747305[12] = 0;
   out_2984795699500747305[13] = 0;
   out_2984795699500747305[14] = 0;
   out_2984795699500747305[15] = 0;
   out_2984795699500747305[16] = 0;
   out_2984795699500747305[17] = 0;
   out_2984795699500747305[18] = 0;
   out_2984795699500747305[19] = 0;
   out_2984795699500747305[20] = 1;
   out_2984795699500747305[21] = 0;
   out_2984795699500747305[22] = 0;
   out_2984795699500747305[23] = 0;
   out_2984795699500747305[24] = 0;
   out_2984795699500747305[25] = 0;
   out_2984795699500747305[26] = 0;
   out_2984795699500747305[27] = 0;
   out_2984795699500747305[28] = 0;
   out_2984795699500747305[29] = 0;
   out_2984795699500747305[30] = 1;
   out_2984795699500747305[31] = 0;
   out_2984795699500747305[32] = 0;
   out_2984795699500747305[33] = 0;
   out_2984795699500747305[34] = 0;
   out_2984795699500747305[35] = 0;
   out_2984795699500747305[36] = 0;
   out_2984795699500747305[37] = 0;
   out_2984795699500747305[38] = 0;
   out_2984795699500747305[39] = 0;
   out_2984795699500747305[40] = 1;
   out_2984795699500747305[41] = 0;
   out_2984795699500747305[42] = 0;
   out_2984795699500747305[43] = 0;
   out_2984795699500747305[44] = 0;
   out_2984795699500747305[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_2984795699500747305[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_2984795699500747305[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2984795699500747305[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2984795699500747305[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_2984795699500747305[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_2984795699500747305[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_2984795699500747305[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_2984795699500747305[53] = -9.8100000000000005*dt;
   out_2984795699500747305[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_2984795699500747305[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_2984795699500747305[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2984795699500747305[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2984795699500747305[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_2984795699500747305[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_2984795699500747305[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_2984795699500747305[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2984795699500747305[62] = 0;
   out_2984795699500747305[63] = 0;
   out_2984795699500747305[64] = 0;
   out_2984795699500747305[65] = 0;
   out_2984795699500747305[66] = 0;
   out_2984795699500747305[67] = 0;
   out_2984795699500747305[68] = 0;
   out_2984795699500747305[69] = 0;
   out_2984795699500747305[70] = 1;
   out_2984795699500747305[71] = 0;
   out_2984795699500747305[72] = 0;
   out_2984795699500747305[73] = 0;
   out_2984795699500747305[74] = 0;
   out_2984795699500747305[75] = 0;
   out_2984795699500747305[76] = 0;
   out_2984795699500747305[77] = 0;
   out_2984795699500747305[78] = 0;
   out_2984795699500747305[79] = 0;
   out_2984795699500747305[80] = 1;
}
void h_25(double *state, double *unused, double *out_4848168669919202886) {
   out_4848168669919202886[0] = state[6];
}
void H_25(double *state, double *unused, double *out_7960279069568034739) {
   out_7960279069568034739[0] = 0;
   out_7960279069568034739[1] = 0;
   out_7960279069568034739[2] = 0;
   out_7960279069568034739[3] = 0;
   out_7960279069568034739[4] = 0;
   out_7960279069568034739[5] = 0;
   out_7960279069568034739[6] = 1;
   out_7960279069568034739[7] = 0;
   out_7960279069568034739[8] = 0;
}
void h_24(double *state, double *unused, double *out_5973332433336352466) {
   out_5973332433336352466[0] = state[4];
   out_5973332433336352466[1] = state[5];
}
void H_24(double *state, double *unused, double *out_8313815405136017311) {
   out_8313815405136017311[0] = 0;
   out_8313815405136017311[1] = 0;
   out_8313815405136017311[2] = 0;
   out_8313815405136017311[3] = 0;
   out_8313815405136017311[4] = 1;
   out_8313815405136017311[5] = 0;
   out_8313815405136017311[6] = 0;
   out_8313815405136017311[7] = 0;
   out_8313815405136017311[8] = 0;
   out_8313815405136017311[9] = 0;
   out_8313815405136017311[10] = 0;
   out_8313815405136017311[11] = 0;
   out_8313815405136017311[12] = 0;
   out_8313815405136017311[13] = 0;
   out_8313815405136017311[14] = 1;
   out_8313815405136017311[15] = 0;
   out_8313815405136017311[16] = 0;
   out_8313815405136017311[17] = 0;
}
void h_30(double *state, double *unused, double *out_7443128207949688601) {
   out_7443128207949688601[0] = state[4];
}
void H_30(double *state, double *unused, double *out_5441946111060786112) {
   out_5441946111060786112[0] = 0;
   out_5441946111060786112[1] = 0;
   out_5441946111060786112[2] = 0;
   out_5441946111060786112[3] = 0;
   out_5441946111060786112[4] = 1;
   out_5441946111060786112[5] = 0;
   out_5441946111060786112[6] = 0;
   out_5441946111060786112[7] = 0;
   out_5441946111060786112[8] = 0;
}
void h_26(double *state, double *unused, double *out_5333477160662303659) {
   out_5333477160662303659[0] = state[7];
}
void H_26(double *state, double *unused, double *out_6744961685267460653) {
   out_6744961685267460653[0] = 0;
   out_6744961685267460653[1] = 0;
   out_6744961685267460653[2] = 0;
   out_6744961685267460653[3] = 0;
   out_6744961685267460653[4] = 0;
   out_6744961685267460653[5] = 0;
   out_6744961685267460653[6] = 0;
   out_6744961685267460653[7] = 1;
   out_6744961685267460653[8] = 0;
}
void h_27(double *state, double *unused, double *out_5010412721985507322) {
   out_5010412721985507322[0] = state[3];
}
void H_27(double *state, double *unused, double *out_8182362745197851896) {
   out_8182362745197851896[0] = 0;
   out_8182362745197851896[1] = 0;
   out_8182362745197851896[2] = 0;
   out_8182362745197851896[3] = 1;
   out_8182362745197851896[4] = 0;
   out_8182362745197851896[5] = 0;
   out_8182362745197851896[6] = 0;
   out_8182362745197851896[7] = 0;
   out_8182362745197851896[8] = 0;
}
void h_29(double *state, double *unused, double *out_8399210544951165364) {
   out_8399210544951165364[0] = state[1];
}
void H_29(double *state, double *unused, double *out_6469000018328300863) {
   out_6469000018328300863[0] = 0;
   out_6469000018328300863[1] = 1;
   out_6469000018328300863[2] = 0;
   out_6469000018328300863[3] = 0;
   out_6469000018328300863[4] = 0;
   out_6469000018328300863[5] = 0;
   out_6469000018328300863[6] = 0;
   out_6469000018328300863[7] = 0;
   out_6469000018328300863[8] = 0;
}
void h_28(double *state, double *unused, double *out_2638932451325827020) {
   out_2638932451325827020[0] = state[0];
}
void H_28(double *state, double *unused, double *out_8432630289893627114) {
   out_8432630289893627114[0] = 1;
   out_8432630289893627114[1] = 0;
   out_8432630289893627114[2] = 0;
   out_8432630289893627114[3] = 0;
   out_8432630289893627114[4] = 0;
   out_8432630289893627114[5] = 0;
   out_8432630289893627114[6] = 0;
   out_8432630289893627114[7] = 0;
   out_8432630289893627114[8] = 0;
}
void h_31(double *state, double *unused, double *out_4572974607634696997) {
   out_4572974607634696997[0] = state[8];
}
void H_31(double *state, double *unused, double *out_6118753583034109177) {
   out_6118753583034109177[0] = 0;
   out_6118753583034109177[1] = 0;
   out_6118753583034109177[2] = 0;
   out_6118753583034109177[3] = 0;
   out_6118753583034109177[4] = 0;
   out_6118753583034109177[5] = 0;
   out_6118753583034109177[6] = 0;
   out_6118753583034109177[7] = 0;
   out_6118753583034109177[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_4611641411917091701) {
  err_fun(nom_x, delta_x, out_4611641411917091701);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_3968532722392907058) {
  inv_err_fun(nom_x, true_x, out_3968532722392907058);
}
void car_H_mod_fun(double *state, double *out_1232909156344202498) {
  H_mod_fun(state, out_1232909156344202498);
}
void car_f_fun(double *state, double dt, double *out_241923337051056220) {
  f_fun(state,  dt, out_241923337051056220);
}
void car_F_fun(double *state, double dt, double *out_2984795699500747305) {
  F_fun(state,  dt, out_2984795699500747305);
}
void car_h_25(double *state, double *unused, double *out_4848168669919202886) {
  h_25(state, unused, out_4848168669919202886);
}
void car_H_25(double *state, double *unused, double *out_7960279069568034739) {
  H_25(state, unused, out_7960279069568034739);
}
void car_h_24(double *state, double *unused, double *out_5973332433336352466) {
  h_24(state, unused, out_5973332433336352466);
}
void car_H_24(double *state, double *unused, double *out_8313815405136017311) {
  H_24(state, unused, out_8313815405136017311);
}
void car_h_30(double *state, double *unused, double *out_7443128207949688601) {
  h_30(state, unused, out_7443128207949688601);
}
void car_H_30(double *state, double *unused, double *out_5441946111060786112) {
  H_30(state, unused, out_5441946111060786112);
}
void car_h_26(double *state, double *unused, double *out_5333477160662303659) {
  h_26(state, unused, out_5333477160662303659);
}
void car_H_26(double *state, double *unused, double *out_6744961685267460653) {
  H_26(state, unused, out_6744961685267460653);
}
void car_h_27(double *state, double *unused, double *out_5010412721985507322) {
  h_27(state, unused, out_5010412721985507322);
}
void car_H_27(double *state, double *unused, double *out_8182362745197851896) {
  H_27(state, unused, out_8182362745197851896);
}
void car_h_29(double *state, double *unused, double *out_8399210544951165364) {
  h_29(state, unused, out_8399210544951165364);
}
void car_H_29(double *state, double *unused, double *out_6469000018328300863) {
  H_29(state, unused, out_6469000018328300863);
}
void car_h_28(double *state, double *unused, double *out_2638932451325827020) {
  h_28(state, unused, out_2638932451325827020);
}
void car_H_28(double *state, double *unused, double *out_8432630289893627114) {
  H_28(state, unused, out_8432630289893627114);
}
void car_h_31(double *state, double *unused, double *out_4572974607634696997) {
  h_31(state, unused, out_4572974607634696997);
}
void car_H_31(double *state, double *unused, double *out_6118753583034109177) {
  H_31(state, unused, out_6118753583034109177);
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
