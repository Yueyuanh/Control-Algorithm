#pragma once
#include "utils.h"

//PID������
class PIDController {
private:
    vector<double> __Kp;
    vector<double> __Ki;
    vector<double> __Kd;
    vector<double> __last_error;
    vector<double> __integral;

    vector<pair<double, double>> __control_limits; // ����Լ��
    vector<pair<double, double>> __integral_limits; // ����Լ��
    vector<double> __feedforward_gain; // ǰ������

public:
    PIDController(
        double Kp, 
        double Ki, 
        double Kd,
        const pair<double, double>& control_limits = {}, // NOTE Ĭ�ϲ���ֻ������һ��, ͷ�ļ�������Դ�ļ��Ͳ�Ҫ����
        const pair<double, double>& integral_limits = {},
        double feedforward_gain = 0.0
        );

    double call(double setpoint, double feedback, double dt, double expected_output = 0.0);
    double operator()(double setpoint, double feedback, double dt, double expected_output = 0.0);

    PIDController(
        const vector<double>& Kp, 
        const vector<double>& Ki, 
        const vector<double>& Kd,
        const vector<pair<double, double>>& control_limits = {},
        const vector<pair<double, double>>& integral_limits = {},
        const vector<double>& feedforward_gain = {}
    );
    
    vector<double> call(const vector<double>& setpoint, const vector<double>& feedback, double dt, const vector<double>& expected_output = {});
    vector<double> operator()(const vector<double>& setpoint, const vector<double>& feedback, double dt, const vector<double>& expected_output = {});

};
