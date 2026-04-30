#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_1608596609641417580);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1300195658449584630);
void pose_H_mod_fun(double *state, double *out_4243197136101237079);
void pose_f_fun(double *state, double dt, double *out_5843270271854622054);
void pose_F_fun(double *state, double dt, double *out_1867205219410655269);
void pose_h_4(double *state, double *unused, double *out_980464455538860381);
void pose_H_4(double *state, double *unused, double *out_471389926257329507);
void pose_h_10(double *state, double *unused, double *out_5036026826105273660);
void pose_H_10(double *state, double *unused, double *out_6410218509014697900);
void pose_h_13(double *state, double *unused, double *out_1633683833944592611);
void pose_H_13(double *state, double *unused, double *out_7139241282059371422);
void pose_h_14(double *state, double *unused, double *out_8904836124963581929);
void pose_H_14(double *state, double *unused, double *out_3491850930082155022);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}