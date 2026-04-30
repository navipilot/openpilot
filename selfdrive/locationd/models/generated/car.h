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
void car_err_fun(double *nom_x, double *delta_x, double *out_5320341998124928693);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_864421799547270951);
void car_H_mod_fun(double *state, double *out_3451790410312107701);
void car_f_fun(double *state, double dt, double *out_4081593184001938296);
void car_F_fun(double *state, double dt, double *out_6533608797065389332);
void car_h_25(double *state, double *unused, double *out_1286712737219592117);
void car_H_25(double *state, double *unused, double *out_71190239677383976);
void car_h_24(double *state, double *unused, double *out_4020914257266522617);
void car_H_24(double *state, double *unused, double *out_3629115838479707112);
void car_h_30(double *state, double *unused, double *out_1561906799504098006);
void car_H_30(double *state, double *unused, double *out_4456506090450224222);
void car_h_26(double *state, double *unused, double *out_7458636612936378436);
void car_H_26(double *state, double *unused, double *out_3670313079196672248);
void car_h_27(double *state, double *unused, double *out_2766640841793939077);
void car_H_27(double *state, double *unused, double *out_2232912019266281005);
void car_h_29(double *state, double *unused, double *out_6371076442889912105);
void car_H_29(double *state, double *unused, double *out_3946274746135832038);
void car_h_28(double *state, double *unused, double *out_7768314109454784722);
void car_H_28(double *state, double *unused, double *out_1982644474570505787);
void car_h_31(double *state, double *unused, double *out_8520153859771417615);
void car_H_31(double *state, double *unused, double *out_4296521181430023724);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}