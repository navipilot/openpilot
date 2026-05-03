#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_5188089899736494345);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_516319331577825853);
void pose_H_mod_fun(double *state, double *out_4951567132479851062);
void pose_f_fun(double *state, double dt, double *out_898454825503606476);
void pose_F_fun(double *state, double dt, double *out_906698228237069812);
void pose_h_4(double *state, double *unused, double *out_6577940567349030778);
void pose_H_4(double *state, double *unused, double *out_913158878120160673);
void pose_h_10(double *state, double *unused, double *out_3373051515203641771);
void pose_H_10(double *state, double *unused, double *out_7288760212554836769);
void pose_h_13(double *state, double *unused, double *out_413669578140119338);
void pose_H_13(double *state, double *unused, double *out_8523790086436861602);
void pose_h_14(double *state, double *unused, double *out_8770101800505378255);
void pose_H_14(double *state, double *unused, double *out_4876399734459645202);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}