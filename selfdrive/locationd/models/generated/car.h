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
void car_err_fun(double *nom_x, double *delta_x, double *out_2689391300508487509);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_7177783815062799236);
void car_H_mod_fun(double *state, double *out_8221602802744378233);
void car_f_fun(double *state, double dt, double *out_7651705913211861388);
void car_F_fun(double *state, double dt, double *out_97859790539154602);
void car_h_25(double *state, double *unused, double *out_1747255377487496462);
void car_H_25(double *state, double *unused, double *out_7588584345949747189);
void car_h_24(double *state, double *unused, double *out_3070396560078381804);
void car_H_24(double *state, double *unused, double *out_7300234128957481291);
void car_h_30(double *state, double *unused, double *out_2594165583737162996);
void car_H_30(double *state, double *unused, double *out_8339826769252555800);
void car_h_26(double *state, double *unused, double *out_1234538052727793168);
void car_H_26(double *state, double *unused, double *out_3847081027075690965);
void car_h_27(double *state, double *unused, double *out_4784041428931829147);
void car_H_27(double *state, double *unused, double *out_886124704021714080);
void car_h_29(double *state, double *unused, double *out_2407507683923682649);
void car_H_29(double *state, double *unused, double *out_3571119360136531175);
void car_h_28(double *state, double *unused, double *out_7679778805471580716);
void car_H_28(double *state, double *unused, double *out_5534749631701857426);
void car_h_31(double *state, double *unused, double *out_5023579848862854474);
void car_H_31(double *state, double *unused, double *out_3220872924842339489);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}