#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_3060982030804230470);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1893009693429340384);
void pose_H_mod_fun(double *state, double *out_8923486826760505000);
void pose_f_fun(double *state, double dt, double *out_6932327335752377632);
void pose_F_fun(double *state, double dt, double *out_3364331378734993426);
void pose_h_4(double *state, double *unused, double *out_2331867789495253821);
void pose_H_4(double *state, double *unused, double *out_7103788100854208139);
void pose_h_10(double *state, double *unused, double *out_6139878679209159095);
void pose_H_10(double *state, double *unused, double *out_1053793302334783175);
void pose_h_13(double *state, double *unused, double *out_1682911552899262681);
void pose_H_13(double *state, double *unused, double *out_3732324764538642548);
void pose_h_14(double *state, double *unused, double *out_7957741524853228127);
void pose_H_14(double *state, double *unused, double *out_4020999668558835843);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}