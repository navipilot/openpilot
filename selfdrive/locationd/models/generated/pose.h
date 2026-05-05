#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_8612662017984698055);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_6583538545281744086);
void pose_H_mod_fun(double *state, double *out_7537065087244600898);
void pose_f_fun(double *state, double dt, double *out_6289537546199484080);
void pose_F_fun(double *state, double dt, double *out_3263235949829739422);
void pose_h_4(double *state, double *unused, double *out_6871661940623032085);
void pose_H_4(double *state, double *unused, double *out_5061597179712991997);
void pose_h_10(double *state, double *unused, double *out_974669649895256171);
void pose_H_10(double *state, double *unused, double *out_3862977066975629010);
void pose_h_13(double *state, double *unused, double *out_2719357598944466940);
void pose_H_13(double *state, double *unused, double *out_8273871005045324798);
void pose_h_14(double *state, double *unused, double *out_5270922229605318979);
void pose_H_14(double *state, double *unused, double *out_9024838036052476526);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}