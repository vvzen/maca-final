# @desc: this command line app takes as argument the mass of the object you want to lift
# and returns the required torque for a 5mm shaft stepper motor
# @args: -w, the mass (kg) of the object to move upwards
#        -t, the torque (kg/cm) of the stepper motor
# @usage: python compute_torque_for_weight.py -w 10

import argparse

# constants
SHAFT_RADIUS = 0.05 # m
GRAVITATIONAL_ATTRACTION = 9.80665 # m/s^2
REQUIRED_ACCELERATION = 3 # m/s^2

# construct the argument parse and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-w", "--weight", required=True, type=float, help="mass (kg) of the weight to lift")
ap.add_argument("-t", "--torque", required=True, type=float, help="torque (kg/cm) of your stepper motor")
args = vars(ap.parse_args())

mass = args["weight"]
stepper_torque = args["torque"]
print("your object mass is {} kg".format(mass))

# 1. compute force of gravity on object
# F = ma
force = mass * GRAVITATIONAL_ATTRACTION

print('1. you have a {}N force pulling the aluminium bar down'.format(force))
 
# 2. compute torque required to keep the system in a steady state
# Torque = force * radius
steady_torque = force * SHAFT_RADIUS

print('2. you need a {}Nm torque to keep the system steady'.format(steady_torque))

# 3. to get the real torque you have to specify how fast you want to accelerate
up_force = mass * REQUIRED_ACCELERATION
up_torque = up_force * SHAFT_RADIUS

# 4. this the force we need to fight gravity and push the object upwards
total_force = up_force + force
total_torque = total_force * SHAFT_RADIUS
# 1 newton meter is equal to 10.197162129779 kg-cm.
total_torque_kgcm = total_torque * 10.197162129779

print('3. you need a:')
print('\t{}N force to move the object up'.format(total_force))
print('\t{}Nm torque to move the object up'.format(total_torque))
print('\t{}Kg/cm torque to move the object up'.format(total_torque_kgcm))

print('\nyour motor has a {}Kg/cm torque'.format(stepper_torque))
