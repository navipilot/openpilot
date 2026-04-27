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
void car_err_fun(double *nom_x, double *delta_x, double *out_4898271858899960872);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_6923660614592572242);
void car_H_mod_fun(double *state, double *out_2317713943645087999);
void car_f_fun(double *state, double dt, double *out_7025688067405742044);
void car_F_fun(double *state, double dt, double *out_8992610323869343845);
void car_h_25(double *state, double *unused, double *out_659508963430133761);
void car_H_25(double *state, double *unused, double *out_5411732216437484507);
void car_h_24(double *state, double *unused, double *out_2182025158618031287);
void car_H_24(double *state, double *unused, double *out_6824760464475893344);
void car_h_30(double *state, double *unused, double *out_3607234953785618722);
void car_H_30(double *state, double *unused, double *out_7930065174944733134);
void car_h_26(double *state, double *unused, double *out_6419787057055472105);
void car_H_26(double *state, double *unused, double *out_1670228897563428283);
void car_h_27(double *state, double *unused, double *out_1197958375544545365);
void car_H_27(double *state, double *unused, double *out_5755301863144308223);
void car_h_29(double *state, double *unused, double *out_8608908221446464367);
void car_H_29(double *state, double *unused, double *out_8440296519259125318);
void car_h_28(double *state, double *unused, double *out_7519878544324146961);
void car_H_28(double *state, double *unused, double *out_3357897502189594744);
void car_h_31(double *state, double *unused, double *out_7221763687323915424);
void car_H_31(double *state, double *unused, double *out_1044020795330076807);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}