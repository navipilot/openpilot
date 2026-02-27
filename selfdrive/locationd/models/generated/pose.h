#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_8902926383898082118);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_4257102035919230637);
void pose_H_mod_fun(double *state, double *out_4577645484999217247);
void pose_f_fun(double *state, double dt, double *out_8384727316265601151);
void pose_F_fun(double *state, double dt, double *out_26276543904169635);
void pose_h_4(double *state, double *unused, double *out_7158146674662545747);
void pose_H_4(double *state, double *unused, double *out_8716580203671253634);
void pose_h_10(double *state, double *unused, double *out_5318715798257585615);
void pose_H_10(double *state, double *unused, double *out_3564541191444468409);
void pose_h_13(double *state, double *unused, double *out_8548651950161202149);
void pose_H_13(double *state, double *unused, double *out_5504306378338920833);
void pose_h_14(double *state, double *unused, double *out_313380574240030784);
void pose_H_14(double *state, double *unused, double *out_4753339347331769105);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}