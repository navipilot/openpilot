#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_4055321504301233767);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_5242087564548968290);
void pose_H_mod_fun(double *state, double *out_6187186838300588967);
void pose_f_fun(double *state, double dt, double *out_8843286105500853100);
void pose_F_fun(double *state, double dt, double *out_4695420932330585758);
void pose_h_4(double *state, double *unused, double *out_2276007570038364899);
void pose_H_4(double *state, double *unused, double *out_339105031548630246);
void pose_h_10(double *state, double *unused, double *out_530488663462245196);
void pose_H_10(double *state, double *unused, double *out_3914913893563359029);
void pose_h_13(double *state, double *unused, double *out_7290329458186336837);
void pose_H_13(double *state, double *unused, double *out_3551378856880963047);
void pose_h_14(double *state, double *unused, double *out_9018381316782859541);
void pose_H_14(double *state, double *unused, double *out_4302345887888114775);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}