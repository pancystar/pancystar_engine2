#include"pancy_physx.h"
//physx基础类
pancy_physx_basic *pancy_physx_basic::physxbasic_pInstance = NULL;
pancy_physx_basic::pancy_physx_basic()
{
	physic_device = NULL;
	foundation_need = NULL;
}
void pancy_physx_basic::release()
{
	physic_device->release();
	foundation_need->release();
}
engine_basic::engine_fail_reason pancy_physx_basic::create()
{
	physx::PxDefaultErrorCallback error_message;
	foundation_need = PxCreateFoundation(PX_PHYSICS_VERSION, allocator_defaule, error_message);
	physic_device = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation_need, scale);
	if (physic_device == NULL)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "create physx device error");
		return error_message;
	}
	if (physic_device->getPvdConnectionManager())
	{
		const char* pvd_host_ip = "127.0.0.1";
		int port = 5425;
		const int timeout = 100;
		physx::PxVisualDebuggerConnectionFlags connectflag = physx::PxVisualDebuggerExt::getAllConnectionFlags();
		physx::debugger::comm::PvdConnection *theconnection = physx::PxVisualDebuggerExt::createConnection(physic_device->getPvdConnectionManager(), pvd_host_ip, port, timeout, connectflag);
		if (theconnection)
		{
			MessageBox(0, L"debug connection success", L"tip", MB_OK);
		}
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//physx场景类
pancy_physx_scene::pancy_physx_scene()
{
	//physic_device = NULL;
	//foundation_need = NULL;
	now_scene = NULL;
	controller_manager = NULL;
	
	dynamic_object_autoincres_ID = static_cast<unsigned __int64>(0);
	static_object_autoincres_ID = static_cast<unsigned __int64>(0);
	charactor_object_autoincres_ID = static_cast<unsigned __int64>(0);
	terrain_object_autoincres_ID = static_cast<unsigned __int64>(0);
	//player = NULL;
}
engine_basic::engine_fail_reason pancy_physx_scene::create()
{
	physic_device = pancy_physx_basic::GetInstance()->get_device();
	/*
	physx::PxDefaultErrorCallback error_message;
	foundation_need = PxCreateFoundation(PX_PHYSICS_VERSION, allocator_defaule, error_message);
	physic_device = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation_need, scale);
	if (physic_device == NULL)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "create physx device error");
		return error_message;
	}
	
	if (physic_device->getPvdConnectionManager())
	{
		const char* pvd_host_ip = "127.0.0.1";
		int port = 5425;
		const int timeout = 100;
		physx::PxVisualDebuggerConnectionFlags connectflag = physx::PxVisualDebuggerExt::getAllConnectionFlags();
		physx::debugger::comm::PvdConnection *theconnection = physx::PxVisualDebuggerExt::createConnection(physic_device->getPvdConnectionManager(), pvd_host_ip, port, timeout, connectflag);
		if (theconnection)
		{
			MessageBox(0, L"debug connection success", L"tip", MB_OK);
		}
	}
*/
	physx::PxSimulationEventCallback *callback_scene = new pancy_collision_callback();
	physx::PxSceneDesc scene_desc(physic_device->getTolerancesScale());
	scene_desc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
	scene_desc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	scene_desc.filterShader = physx::PxDefaultSimulationFilterShader;
	scene_desc.simulationEventCallback = callback_scene;

	now_scene = physic_device->createScene(scene_desc);

	controller_manager = PxCreateControllerManager(*now_scene);
	if (controller_manager == NULL) 
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "create controller manager error");
		return error_message;
	}
	terrain_mat_force = physic_device->createMaterial(0.5, 0.5, 0.5);
	/*
	physx::PxCapsuleControllerDesc test_capsule_desc;
	test_capsule_desc.position = physx::PxExtendedVec3(0.0f,200.0f,0.0f);
	test_capsule_desc.contactOffset = 0.05f;
	test_capsule_desc.stepOffset = 0.01;
	test_capsule_desc.slopeLimit = 0.5f;
	test_capsule_desc.radius = 0.5f;
	test_capsule_desc.height = 2;
	test_capsule_desc.upDirection = physx::PxVec3(0,1,0);
	physx::PxMaterial *mat_force = physic_device->createMaterial(0.5, 0.5, 0.5);
	test_capsule_desc.material = mat_force;
	test_capsule_desc.maxJumpHeight = 5.0f;
	bool rec_check = test_capsule_desc.isValid();
	player = controller_manager->createController(test_capsule_desc);
	*/
	/*
	plane = physic_device->createRigidStatic(*plan_pos);
	plane->createShape(physx::PxPlaneGeometry(), *mat_force);
	//now_scene->addActor(*plane);
	*/
	//physx::PxShape *trigger;
	//plane->getShapes(&trigger, 1);
	//trigger->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	//trigger->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	/*
	box = physx::PxCreateDynamic(*physic_device, *box_pos, *box_geo, *mat_force, 1);
	now_scene->addActor(*box);
	//box->setMass(0.2f);
	physx::PxReal rec = box->getMass();
	physx::PxVec3 dir_force(19.0f, 0.0f, 0.0f);
	box->addForce(dir_force, physx::PxForceMode::eIMPULSE);
	*/
	//physx::PxShape *trigger1;
	//box->getShapes(&trigger1, 1);
	//trigger1->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	//trigger1->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::add_a_terrain_object(
	physx::PxHeightFieldDesc height_data, 
	physx::PxVec3 scal_data, 
	physx::PxTransform terrain_trans, 
	unsigned __int64 &object_ID
	)
{
	physx::PxHeightField* aHeightField = physic_device->createHeightField(height_data);
	physx::PxHeightFieldGeometry hfGeom(aHeightField, physx::PxMeshGeometryFlags(), scal_data.x, scal_data.y, scal_data.z);
	physx::PxRigidStatic* aHeightFieldActor = physic_device->createRigidStatic(terrain_trans);
	if (aHeightFieldActor == NULL) 
	{
		engine_basic::engine_fail_reason error_message(E_FAIL,"create terrain height map physic actor error");
		return error_message;
	}
	physx::PxShape* aHeightFieldShape = aHeightFieldActor->createShape(hfGeom, *terrain_mat_force);
	if (aHeightFieldShape == NULL)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "build terrain height map physic shape error");
		return error_message;
	}
	std::pair<unsigned __int64, physx::PxRigidStatic*> now_terrain(terrain_object_autoincres_ID, aHeightFieldActor);
	terrain_object_list.insert(now_terrain);
	object_ID = terrain_object_autoincres_ID;
	terrain_object_autoincres_ID += 1;
	now_scene->addActor(*aHeightFieldActor);


	//aHeightFieldActor->detachShape(*aHeightFieldShape);
	aHeightField->release();
	
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::add_a_dynamic_object(
	physx::PxTransform position_st, 
	physx::PxGeometry geometry_need, 
	XMFLOAT3 mat_need,
	unsigned __int64 &object_ID
	)
{
	physx::PxMaterial *mat_physx = physic_device->createMaterial(mat_need.x, mat_need.y, mat_need.z);
	physx::PxRigidDynamic *dynamic_need;
	dynamic_need = physx::PxCreateDynamic(*physic_device, position_st, geometry_need, *mat_physx, 1);
	if (dynamic_need == NULL)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL,"Create Dynamic Object error");
		return error_message;
	}
	std::pair<unsigned __int64, physx::PxRigidDynamic*> now_dynamic(dynamic_object_autoincres_ID, dynamic_need);
	dynamic_object_list.insert(now_dynamic);
	object_ID = dynamic_object_autoincres_ID;
	dynamic_object_autoincres_ID += 1;
	engine_basic::engine_fail_reason succeed;
	return succeed;
	/*
	//now_scene->addActor(*box_need);
	//box->setMass(0.2f);
	physx::PxReal rec = box_need->getMass();
	physx::PxVec3 dir_force(19.0f, 0.0f, 0.0f);
	box_need->addForce(dir_force, physx::PxForceMode::eIMPULSE);
	*physic_out = box_need;
	return S_OK;
	*/
}
engine_basic::engine_fail_reason pancy_physx_scene::add_a_static_object(
	physx::PxTransform position_st,
	physx::PxGeometry geometry_need,
	physx::PxMaterial *mat_need,
	unsigned __int64 &object_ID
	) 
{
	physx::PxRigidStatic *static_need;
	static_need = physx::PxCreateStatic(*physic_device, position_st, geometry_need, *mat_need);
	if (static_need == NULL)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "Create Static Object error");
		return error_message;
	}
	std::pair<unsigned __int64, physx::PxRigidStatic*> now_static(static_object_autoincres_ID, static_need);
	static_object_list.insert(now_static);
	object_ID = static_object_autoincres_ID;
	static_object_autoincres_ID += 1;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::add_a_charactor_object(physx::PxControllerDesc *charactor_desc,unsigned __int64 &object_ID)
{
	auto charactor_out = controller_manager->createController(*charactor_desc);
	if (charactor_out == NULL)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL,"add charactor error");
		return error_message;
	}
	std::pair<unsigned __int64, physx::PxController*> now_charactor(charactor_object_autoincres_ID, charactor_out);
	charactor_object_list.insert(now_charactor);
	object_ID = charactor_object_autoincres_ID;
	charactor_object_autoincres_ID += 1;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason pancy_physx_scene::wakeup_a_terrain(unsigned __int64 object_ID)
{
	auto terrain_now = terrain_object_list.find(object_ID);
	if (terrain_now == terrain_object_list.end()) 
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the terrain");
	}
	now_scene->addActor(*terrain_now->second);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::sleep_a_terrain(unsigned __int64 object_ID)
{
	auto terrain_now = terrain_object_list.find(object_ID);
	if (terrain_now == terrain_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the terrain");
	}
	now_scene->removeActor(*terrain_now->second);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::delete_a_terrain(unsigned __int64 object_ID)
{
	auto terrain_now = terrain_object_list.find(object_ID);
	if (terrain_now == terrain_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the terrain");
	}
	sleep_a_terrain(object_ID);
	terrain_now->second->release();
	terrain_object_list.erase(terrain_now);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason pancy_physx_scene::wakeup_a_dynamic(unsigned __int64 object_ID)
{
	auto dynamic_now = dynamic_object_list.find(object_ID);
	if (dynamic_now == dynamic_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the dynamic");
	}
	now_scene->addActor(*dynamic_now->second);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::sleep_a_dynamic(unsigned __int64 object_ID)
{
	auto dynamic_now = dynamic_object_list.find(object_ID);
	if (dynamic_now == dynamic_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the dynamic");
	}
	now_scene->removeActor(*dynamic_now->second);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::delete_a_dynamic(unsigned __int64 object_ID)
{
	auto dynamic_now = dynamic_object_list.find(object_ID);
	if (dynamic_now == dynamic_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the dynamic");
	}
	sleep_a_dynamic(object_ID);
	dynamic_now->second->release();
	dynamic_object_list.erase(dynamic_now);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason pancy_physx_scene::wakeup_a_static(unsigned __int64 object_ID)
{
	auto static_now = static_object_list.find(object_ID);
	if (static_now == static_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the static");
	}
	now_scene->addActor(*static_now->second);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::sleep_a_static(unsigned __int64 object_ID)
{
	auto static_now = static_object_list.find(object_ID);
	if (static_now == static_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the static");
	}
	now_scene->removeActor(*static_now->second);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::delete_a_static(unsigned __int64 object_ID)
{
	auto static_now = static_object_list.find(object_ID);
	if (static_now == static_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the static");
	}
	sleep_a_static(object_ID);
	static_now->second->release();
	static_object_list.erase(static_now);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason pancy_physx_scene::wakeup_a_charactor(unsigned __int64 object_ID)
{
	auto charactor_now = charactor_object_list.find(object_ID);
	if (charactor_now == charactor_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the charactor");
	}
	auto actor_dynamic = charactor_now->second->getActor();
	now_scene->addActor(*actor_dynamic);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::sleep_a_charactor(unsigned __int64 object_ID)
{
	auto charactor_now = charactor_object_list.find(object_ID);
	if (charactor_now == charactor_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the charactor");
	}
	auto actor_dynamic = charactor_now->second->getActor();
	now_scene->removeActor(*actor_dynamic);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::delete_a_charactor(unsigned __int64 object_ID)
{
	auto charactor_now = charactor_object_list.find(object_ID);
	if (charactor_now == charactor_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the charactor");
	}
	sleep_a_static(object_ID);
	charactor_now->second->release();
	charactor_object_list.erase(charactor_now);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

/*
HRESULT pancy_physx::create_charactor(physx::PxCapsuleControllerDesc charactor_desc, physx::PxController **charactor_out)
{

	*charactor_out = controller_manager->createController(charactor_desc);
	if (*charactor_out == NULL) 
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT pancy_physx::create_charactor(physx::PxBoxControllerDesc charactor_desc, physx::PxController **charactor_out)
{
	*charactor_out = controller_manager->createController(charactor_desc);
	if (*charactor_out == NULL)
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT pancy_physx::add_actor(physx::PxRigidDynamic *box)
{
	now_scene->addActor(*box);
	return S_OK;
}
*/
engine_basic::engine_fail_reason pancy_physx_scene::get_position_dynamic(unsigned __int64 object_ID, XMFLOAT3 &position)
{
	auto dynamic_now = dynamic_object_list.find(object_ID);
	if (dynamic_now == dynamic_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the dynamic");
	}
	position.x = static_cast<float>(dynamic_now->second->getGlobalPose().p.x);
	position.y = static_cast<float>(dynamic_now->second->getGlobalPose().p.y);
	position.z = static_cast<float>(dynamic_now->second->getGlobalPose().p.z);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::get_rotation_dynamic(unsigned __int64 object_ID, float &angle, XMFLOAT3 &vector)
{
	physx::PxReal angle_need;
	physx::PxVec3 vector_need;
	auto dynamic_now = dynamic_object_list.find(object_ID);
	if (dynamic_now == dynamic_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the dynamic");
	}
	dynamic_now->second->getGlobalPose().q.toRadiansAndUnitAxis(angle_need, vector_need);
	angle = static_cast<float>(angle_need);
	vector.x = static_cast<float>(vector_need.x);
	vector.y = static_cast<float>(vector_need.y);
	vector.z = static_cast<float>(vector_need.z);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::get_position_charactor(unsigned __int64 object_ID, XMFLOAT3 &position)
{
	auto charactor_now = charactor_object_list.find(object_ID);
	if (charactor_now == charactor_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the charactor");
	}
	physx::PxExtendedVec3 rec_pos = charactor_now->second->getPosition();
	position.x = rec_pos.x;
	position.y = rec_pos.y;
	position.z = rec_pos.z;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_physx_scene::move_a_charactor(unsigned __int64 object_ID, XMFLOAT3 speed, float delta_time, float min_dist)
{
	auto charactor_now = charactor_object_list.find(object_ID);
	if (charactor_now == charactor_object_list.end())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the charactor");
	}
	physx::PxControllerFilters filters;
	physx::PxVec3 disp = physx::PxVec3(speed.x, -9.8f, speed.z);
	charactor_now->second->move(disp, min_dist, delta_time, filters);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
/*
void pancy_physx::get_rotation_data(physx::PxRigidDynamic *physic_body_in,float &angle, XMFLOAT3 &vector)
{
	physx::PxReal angle_need;
	physx::PxVec3 vector_need;
	physic_body_in->getGlobalPose().q.toRadiansAndUnitAxis(angle_need, vector_need);
	angle = static_cast<float>(angle_need);
	vector.x = static_cast<float>(vector_need.x);
	vector.y = static_cast<float>(vector_need.y);
	vector.z = static_cast<float>(vector_need.z);
}
XMFLOAT3 pancy_physx::get_position(physx::PxRigidDynamic *physic_body_in)
{
	XMFLOAT3 rec_pos;
	rec_pos.x = static_cast<float>(physic_body_in->getGlobalPose().p.x);
	rec_pos.y = static_cast<float>(physic_body_in->getGlobalPose().p.y);
	rec_pos.z = static_cast<float>(physic_body_in->getGlobalPose().p.z);
	return rec_pos;
}
*/
void pancy_physx_scene::update(float delta_time)
{
	//physx::PxReal rec = box->getMass();
	//physx::PxVec3 dir_force(0.0f, 9.8f, 0.0f);
	//box->addForce(dir_force, physx::PxForceMode::eIMPULSE);
	if (delta_time > 0.001f) 
	{
		now_scene->simulate(static_cast<physx::PxReal>(delta_time));
		now_scene->fetchResults(true);
	}
	
	
}
void pancy_physx_scene::release()
{
	now_scene->release();
	now_scene = NULL;
	//physic_device->release();
	//foundation_need->release();
}
