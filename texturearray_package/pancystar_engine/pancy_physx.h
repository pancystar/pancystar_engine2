#pragma once
#include <PxPhysicsAPI.h>
#include<vehicle/PxVehicleSDK.h>
#include<DDSTextureLoader.h>
#include <unordered_map>
#include"pancy_d3d11_basic.h"
#ifdef _DEBUG
#pragma comment(lib,"PhysX3DEBUG_x86.lib")
#pragma comment(lib,"PhysX3CommonDEBUG_x86.lib")
#pragma comment(lib,"PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib,"PhysXVisualDebuggerSDKDEBUG.lib")
#pragma comment(lib,"PhysX3CharacterKinematicDEBUG_x86.lib")
#else
#pragma comment(lib,"PhysX3_x86.lib")
#pragma comment(lib,"PhysX3Common_x86.lib")
#pragma comment(lib,"PhysX3Extensions.lib")
#pragma comment(lib,"PhysXVisualDebuggerSDK.lib")
#pragma comment(lib,"PhysX3CharacterKinematic_x86.lib")
#endif
class pancy_collision_callback : public physx::PxSimulationEventCallback
{
public:
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {};
	void onWake(physx::PxActor** actors, physx::PxU32 count) {};
	void onSleep(physx::PxActor** actors, physx::PxU32 count) {}
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		int a = 0;
	}
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		int a = 0;
	};
};
class pancy_physx_basic 
{
	physx::PxPhysics* physic_device;
	physx::PxFoundation *foundation_need;
	physx::PxDefaultAllocator allocator_defaule;
	physx::PxTolerancesScale scale;
	static pancy_physx_basic *physxbasic_pInstance;
private:
	pancy_physx_basic();
public:
	static engine_basic::engine_fail_reason single_create()
	{
		if (physxbasic_pInstance != NULL)
		{
			engine_basic::engine_fail_reason failed_reason("the d3d basic instance have been created before");
			return failed_reason;
		}
		else
		{
			physxbasic_pInstance = new pancy_physx_basic();
			engine_basic::engine_fail_reason check_failed = physxbasic_pInstance->create();
			return check_failed;
		}
	}
	physx::PxPhysics* get_device() { return physic_device; };
	engine_basic::engine_fail_reason create();
	static pancy_physx_basic *GetInstance()
	{
		return physxbasic_pInstance;
	}
	void release();
};

class pancy_physx_scene
{
	unsigned __int64 dynamic_object_autoincres_ID;
	std::unordered_map<unsigned __int64, physx::PxRigidDynamic*> dynamic_object_list; //动态刚体列表
	unsigned __int64 static_object_autoincres_ID;
	std::unordered_map<unsigned __int64, physx::PxRigidStatic*> static_object_list;   //静态碰撞体列表
	unsigned __int64 charactor_object_autoincres_ID;
	std::unordered_map<unsigned __int64, physx::PxController*> charactor_object_list; //角色动画列表
	unsigned __int64 terrain_object_autoincres_ID;
	std::unordered_map<unsigned __int64, physx::PxRigidStatic*> terrain_object_list;   //地形碰撞体列表
	physx::PxPhysics* physic_device;
	physx::PxScene *now_scene;
	physx::PxControllerManager *controller_manager;
	//内置摩擦材质
	physx::PxMaterial *terrain_mat_force;
public:
	pancy_physx_scene();
	engine_basic::engine_fail_reason create();
	void update(float delta_time);
	void release();
	engine_basic::engine_fail_reason add_a_terrain_object(
		physx::PxHeightFieldDesc height_data, 
		physx::PxVec3 scal_data, 
		physx::PxTransform terrain_trans, 
		unsigned __int64 &object_ID
		);
	engine_basic::engine_fail_reason add_a_dynamic_object(
		physx::PxTransform position_st,
		physx::PxGeometry geometry_need,
		XMFLOAT3 mat_need,
		unsigned __int64 &object_ID
		);
	engine_basic::engine_fail_reason add_a_static_object(
		physx::PxTransform position_st,
		physx::PxGeometry geometry_need,
		physx::PxMaterial *mat_need,
		unsigned __int64 &object_ID
		);
	engine_basic::engine_fail_reason add_a_charactor_object(
		physx::PxControllerDesc *charactor_desc,
		unsigned __int64 &object_ID
		);
	engine_basic::engine_fail_reason wakeup_a_terrain(unsigned __int64 object_ID);
	engine_basic::engine_fail_reason sleep_a_terrain(unsigned __int64 object_ID);
	engine_basic::engine_fail_reason delete_a_terrain(unsigned __int64 object_ID);

	engine_basic::engine_fail_reason wakeup_a_dynamic(unsigned __int64 object_ID);
	engine_basic::engine_fail_reason sleep_a_dynamic(unsigned __int64 object_ID);
	engine_basic::engine_fail_reason delete_a_dynamic(unsigned __int64 object_ID);

	engine_basic::engine_fail_reason wakeup_a_static(unsigned __int64 object_ID);
	engine_basic::engine_fail_reason sleep_a_static(unsigned __int64 object_ID);
	engine_basic::engine_fail_reason delete_a_static(unsigned __int64 object_ID);

	engine_basic::engine_fail_reason wakeup_a_charactor(unsigned __int64 object_ID);
	engine_basic::engine_fail_reason sleep_a_charactor(unsigned __int64 object_ID);
	engine_basic::engine_fail_reason delete_a_charactor(unsigned __int64 object_ID);

	engine_basic::engine_fail_reason get_position_dynamic(unsigned __int64 object_ID,XMFLOAT3 &position);
	engine_basic::engine_fail_reason get_rotation_dynamic(unsigned __int64 object_ID, float &angle, XMFLOAT3 &vector);
	engine_basic::engine_fail_reason get_position_charactor(unsigned __int64 object_ID, XMFLOAT3 &position);
	engine_basic::engine_fail_reason move_a_charactor(unsigned __int64 object_ID, XMFLOAT3 speed,float delta_time,float min_dist = 0.01);
};