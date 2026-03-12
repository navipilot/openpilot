#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_2843212773844661672);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_2943105406875713938);
void pose_H_mod_fun(double *state, double *out_2147512711416992695);
void pose_f_fun(double *state, double dt, double *out_9076294150866414245);
void pose_F_fun(double *state, double dt, double *out_4995496673877862289);
void pose_h_4(double *state, double *unused, double *out_7356118181286553510);
void pose_H_4(double *state, double *unused, double *out_3967211437323289556);
void pose_h_10(double *state, double *unused, double *out_320685202434969677);
void pose_H_10(double *state, double *unused, double *out_156638784834038850);
void pose_h_13(double *state, double *unused, double *out_4281107234856404284);
void pose_H_13(double *state, double *unused, double *out_7800966900625813580);
void pose_h_14(double *state, double *unused, double *out_3917712072833752253);
void pose_H_14(double *state, double *unused, double *out_7049999869618661852);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}