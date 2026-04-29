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
void car_err_fun(double *nom_x, double *delta_x, double *out_4530123797725818472);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_8174571641545541533);
void car_H_mod_fun(double *state, double *out_1010343532093577119);
void car_f_fun(double *state, double dt, double *out_8102414924978153817);
void car_F_fun(double *state, double dt, double *out_1635199146977261472);
void car_h_25(double *state, double *unused, double *out_7182033227359772288);
void car_H_25(double *state, double *unused, double *out_6910994500880282686);
void car_h_24(double *state, double *unused, double *out_8844123752507495678);
void car_H_24(double *state, double *unused, double *out_9166842576004100315);
void car_h_30(double *state, double *unused, double *out_6906839165075266399);
void car_H_30(double *state, double *unused, double *out_2383298170752674488);
void car_h_26(double *state, double *unused, double *out_6538231640973439572);
void car_H_26(double *state, double *unused, double *out_3169491182006226462);
void car_h_27(double *state, double *unused, double *out_8125543080261434723);
void car_H_27(double *state, double *unused, double *out_208534858952249577);
void car_h_29(double *state, double *unused, double *out_7850349017976928834);
void car_H_29(double *state, double *unused, double *out_2893529515067066672);
void car_h_28(double *state, double *unused, double *out_3123869812600589078);
void car_H_28(double *state, double *unused, double *out_2188869502002463902);
void car_h_31(double *state, double *unused, double *out_6770159118416154415);
void car_H_31(double *state, double *unused, double *out_2543283079772874986);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}