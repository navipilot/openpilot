#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_7352766374165151156);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_5943722919136625654);
void pose_H_mod_fun(double *state, double *out_2863715751586478773);
void pose_f_fun(double *state, double dt, double *out_4163803070138990822);
void pose_F_fun(double *state, double dt, double *out_3325085581577154625);
void pose_h_4(double *state, double *unused, double *out_1824245269356126422);
void pose_H_4(double *state, double *unused, double *out_908091458257428799);
void pose_h_10(double *state, double *unused, double *out_5992945801677381357);
void pose_H_10(double *state, double *unused, double *out_1551783764236526040);
void pose_h_13(double *state, double *unused, double *out_4650572533653388734);
void pose_H_13(double *state, double *unused, double *out_1472693377939272903);
void pose_h_14(double *state, double *unused, double *out_9091688560146400700);
void pose_H_14(double *state, double *unused, double *out_2174696974037943497);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}