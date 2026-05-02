#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_1677822181415374238);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1645070502606612381);
void pose_H_mod_fun(double *state, double *out_6168804416891607091);
void pose_f_fun(double *state, double dt, double *out_4562479811319953042);
void pose_F_fun(double *state, double dt, double *out_1994404627029761325);
void pose_h_4(double *state, double *unused, double *out_8379183754462585357);
void pose_H_4(double *state, double *unused, double *out_3264955236904260033);
void pose_h_10(double *state, double *unused, double *out_694976072985625952);
void pose_H_10(double *state, double *unused, double *out_5824925052510502657);
void pose_h_13(double *state, double *unused, double *out_1759161301712070188);
void pose_H_13(double *state, double *unused, double *out_52681411571927232);
void pose_h_14(double *state, double *unused, double *out_9193330309912018009);
void pose_H_14(double *state, double *unused, double *out_698285619435224496);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}