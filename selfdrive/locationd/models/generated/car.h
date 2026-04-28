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
void car_err_fun(double *nom_x, double *delta_x, double *out_700264796655131929);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_107082048498982835);
void car_H_mod_fun(double *state, double *out_7423992576013844798);
void car_f_fun(double *state, double dt, double *out_1496483947477544324);
void car_F_fun(double *state, double dt, double *out_5799168133787523639);
void car_h_25(double *state, double *unused, double *out_4510188062326144511);
void car_H_25(double *state, double *unused, double *out_1340165284045423799);
void car_h_24(double *state, double *unused, double *out_9153409952546890921);
void car_H_24(double *state, double *unused, double *out_4898090882847746935);
void car_h_30(double *state, double *unused, double *out_3253077194494527930);
void car_H_30(double *state, double *unused, double *out_3858498242552672426);
void car_h_26(double *state, double *unused, double *out_3957832291759693581);
void car_H_26(double *state, double *unused, double *out_2401338034828632425);
void car_h_27(double *state, double *unused, double *out_8058270523824692017);
void car_H_27(double *state, double *unused, double *out_1683734930752247515);
void car_h_29(double *state, double *unused, double *out_4420820995570669667);
void car_H_29(double *state, double *unused, double *out_4368729586867064610);
void car_h_28(double *state, double *unused, double *out_7076186430489393905);
void car_H_28(double *state, double *unused, double *out_713669430202465964);
void car_h_31(double *state, double *unused, double *out_8265185813124548370);
void car_H_31(double *state, double *unused, double *out_3027546137061983901);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}