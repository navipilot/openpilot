#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_7099155186075195158);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_2441097136385557776);
void pose_H_mod_fun(double *state, double *out_3208607372720185681);
void pose_f_fun(double *state, double dt, double *out_6464550598295734542);
void pose_F_fun(double *state, double dt, double *out_4669200903438633107);
void pose_h_4(double *state, double *unused, double *out_5330802774113093904);
void pose_H_4(double *state, double *unused, double *out_2454446370665473074);
void pose_h_10(double *state, double *unused, double *out_6487917254077376695);
void pose_H_10(double *state, double *unused, double *out_63197332932821273);
void pose_h_13(double *state, double *unused, double *out_7869262712734505763);
void pose_H_13(double *state, double *unused, double *out_5156184837651227855);
void pose_h_14(double *state, double *unused, double *out_3486024826794941099);
void pose_H_14(double *state, double *unused, double *out_5537234802960845370);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}