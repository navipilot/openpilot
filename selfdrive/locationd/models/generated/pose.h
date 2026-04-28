#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_5792379923597347601);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_3127222841440964837);
void pose_H_mod_fun(double *state, double *out_1474962527938332194);
void pose_f_fun(double *state, double dt, double *out_8107096558916273409);
void pose_F_fun(double *state, double dt, double *out_3803993622350295832);
void pose_h_4(double *state, double *unused, double *out_2415773389544840251);
void pose_H_4(double *state, double *unused, double *out_4675372429039802218);
void pose_h_10(double *state, double *unused, double *out_2790393475270154159);
void pose_H_10(double *state, double *unused, double *out_6532362201945156818);
void pose_h_13(double *state, double *unused, double *out_7195971841206647407);
void pose_H_13(double *state, double *unused, double *out_2935258779276898711);
void pose_h_14(double *state, double *unused, double *out_6081561149972683319);
void pose_H_14(double *state, double *unused, double *out_712131572700317689);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}