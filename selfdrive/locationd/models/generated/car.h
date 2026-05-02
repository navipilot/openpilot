#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_err_fun(double *nom_x, double *delta_x, double *out_7404715378150782082);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_1067581222048175770);
void car_H_mod_fun(double *state, double *out_7894099950222537712);
void car_f_fun(double *state, double dt, double *out_18918990587173632);
void car_F_fun(double *state, double dt, double *out_164532203047016632);
void car_h_25(double *state, double *unused, double *out_513824514853683092);
void car_H_25(double *state, double *unused, double *out_4371119300233046035);
void car_h_24(double *state, double *unused, double *out_1763325166754368628);
void car_H_24(double *state, double *unused, double *out_813193701430722899);
void car_h_30(double *state, double *unused, double *out_2433901475501801869);
void car_H_30(double *state, double *unused, double *out_4500458247376286105);
void car_h_26(double *state, double *unused, double *out_1451817382166503757);
void car_H_26(double *state, double *unused, double *out_8112622619107102259);
void car_h_27(double *state, double *unused, double *out_8950950386849027759);
void car_H_27(double *state, double *unused, double *out_6675221559176711016);
void car_h_29(double *state, double *unused, double *out_7435574743162647514);
void car_H_29(double *state, double *unused, double *out_8388584286046262049);
void car_h_28(double *state, double *unused, double *out_4494120419199678212);
void car_H_28(double *state, double *unused, double *out_6424954014480935798);
void car_h_31(double *state, double *unused, double *out_6257010711496667844);
void car_H_31(double *state, double *unused, double *out_4340473338356085607);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}