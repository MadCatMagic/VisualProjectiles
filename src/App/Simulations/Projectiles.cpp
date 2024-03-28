#include "App/Simulations/Projectiles.h"
#include "App/SimulationFactory.h"

#include "App/Simulations/AnalyticProjectile.h"
#include "App/Simulations/SimpleProjectile.h"
#include "App/Simulations/ProjectileThroughPoint.h"
#include "App/Simulations/BouncyProjectile.h"
#include "App/Simulations/ProjectileWithDrag.h"

void RegisterProjectiles()
{
    GetSimulationFactory().Register("Analytic Projectile", SimulationBuilder<AnalyticProjectile>);
    GetSimulationFactory().Register("Simple Projectile", SimulationBuilder<SimpleProjectile>);
    GetSimulationFactory().Register("Projectile Through Point", SimulationBuilder<ProjectileThroughPoint>);
    GetSimulationFactory().Register("Bouncy Projectile", SimulationBuilder<BouncyProjectile>);
    GetSimulationFactory().Register("Projectile With Drag", SimulationBuilder<ProjectileWithDrag>);
}
