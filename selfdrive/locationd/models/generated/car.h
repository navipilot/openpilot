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
void car_err_fun(double *nom_x, double *delta_x, double *out_1814247348247547720);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_2896416663387818009);
void car_H_mod_fun(double *state, double *out_3833245775098430450);
void car_f_fun(double *state, double dt, double *out_3477590770207651617);
void car_F_fun(double *state, double dt, double *out_3058811064297665734);
void car_h_25(double *state, double *unused, double *out_845892813044312576);
void car_H_25(double *state, double *unused, double *out_6744590887944551998);
void car_h_24(double *state, double *unused, double *out_410909672515548225);
void car_H_24(double *state, double *unused, double *out_673800745732374895);
void car_h_30(double *state, double *unused, double *out_8727165463798361761);
void car_H_30(double *state, double *unused, double *out_7174456855637391420);
void car_h_26(double *state, double *unused, double *out_2811534710064499425);
void car_H_26(double *state, double *unused, double *out_7960649866890943394);
void car_h_27(double *state, double *unused, double *out_4116487184636980656);
void car_H_27(double *state, double *unused, double *out_9048693146888216979);
void car_h_29(double *state, double *unused, double *out_1915699176115514220);
void car_H_29(double *state, double *unused, double *out_7684688199951783604);
void car_h_28(double *state, double *unused, double *out_6496658557271199005);
void car_H_28(double *state, double *unused, double *out_2602289182882253030);
void car_h_31(double *state, double *unused, double *out_9104190579401499048);
void car_H_31(double *state, double *unused, double *out_7334441764657591918);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}