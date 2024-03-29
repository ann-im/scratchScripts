import asyncio
import time

from viam.robot.client import RobotClient
from viam.rpc.dial import Credentials, DialOptions
from viam.components.movement_sensor import MovementSensor
from viam.components.base import Base, Vector3
from viam.services.motion import MotionClient


async def connect():
    opts = RobotClient.Options.with_api_key(
      api_key='ENV_API_KEY',
      api_key_id='ENV_API_KEY_ID'
    )
    return await RobotClient.at_address('modal-orin-main.th85mxrqh1.viam.cloud', opts)

async def main():
    robot = await connect()

    print('Resources:')
    print(robot.resource_names)
  
    # intermode-base
    intermode_base = Base.from_robot(robot, "intermode-base")
    intermode_base_return_value = await intermode_base.is_moving()
    print(f"intermode-base is_moving return value: {intermode_base_return_value}")

    print("start move")
    linearVec = Vector3(x=0.0, y=300.0, z=0.0)
    angularVec = Vector3(x=0.0, y=0.0, z=0.0)
    await intermode_base.set_velocity(linearVec, angularVec)

    time.sleep(5)
    
    print("stop move")
    linearVec.y = 0.0
    await intermode_base.set_velocity(linearVec, angularVec)
  
    """
    # Note this function requires additional arguments.
    # Please provide valid arguments for function to work properly 
    # builtin
    builtin = MotionClient.from_robot(robot, "builtin")
    builtin_return_value = await builtin.get_pose()
    print(f"builtin get_pose return value: {builtin_return_value}")
    """

    # Don't forget to close the robot when you're done!
    await robot.close()

if __name__ == '__main__':
    asyncio.run(main())
