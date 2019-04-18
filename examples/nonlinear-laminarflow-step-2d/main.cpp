// This code simulates the laminar, incompressible water flow past a step. 
//
// A parabolic normal flow velocity of 1 m/s is forced at the inlet and 
// a zero pressure is imposed at the outlet.
//
// More information for this standard example can be found in:
//
// "Finite element methods for the incompressible Navier-Stokes equations", A. Segal


#include "sparselizardbase.h"


using namespace mathop;

mesh createmesh(double lthin, double hthin, double lthick, double hthick, int nlthin, int nlthick, int nhthin, int nhthick);

void sparselizard(void)
{	
    // Region numbers used in this simulation:
    int fluid = 1, inlet = 2, outlet = 3, skin = 4;
    // Height of the inlet [mm]:
    double hthin = 1e-3;
    mesh mymesh = createmesh(2e-3, hthin, 12e-3, 1e-3, 30, 150, 20, 50);

    // Define the fluid wall (not including the inlet and outlet):
    int wall = regionexclusion(skin, regionunion({inlet,outlet}));

    // Dynamic viscosity of water [Pa.s] and density [kg/m3]:
    double mu = 8.9e-4, rho = 1000;

    // Field v is the flow velocity. It uses nodal shape functions "h1" with two components.
    // Field p is the pressure. Fields x and y are the space coordinates.
    field v("h1xy"), p("h1"), x("x"), y("y");

    // Force the flow velocity to 0 on the wall:
    v.setconstraint(wall);
    // Set a 0 pressure at the outlet:
    p.setconstraint(outlet);

    // Use an order 1 interpolation for p and 2 for v on the fluid region (satisfies the BB condition):
    p.setorder(fluid, 1);
    v.setorder(fluid, 2);

    // Define the weak formulation for incompressible laminar flow:
    formulation laminarflow;

    laminarflow += integral(fluid, predefinedlaminarflow(dof(v), tf(v), v, dof(p), tf(p), mu, rho) );


    // This loop with the above formulation is a Newton iteration:
    int index = 0; double convergence = 1, velocity = 0.1; 
    while (convergence > 1e-10)
    {
        // Slowly increase the velocity for a high Reynolds (1 m/s flow, 1000 Reynolds still converges). 
        if (velocity < 0.299)
            velocity = velocity + 0.008;

        std::cout << "Flow velocity: " << velocity << " m/s" << std::endl;
        // Force the flow velocity at the inlet (quadratic profile w.r.t. the y axis):
        v.setconstraint(inlet, array2x1(velocity*y*(hthin-y)/pow(hthin*0.5,2) ,0));

        // Get a measure of the solution for convergence evaluation:
        double measuresol = norm(v).integrate(fluid,2);

        // Generate and solve the laminar flow problem then save to the fields:
        solve(laminarflow);

        convergence = std::abs((norm(v).integrate(fluid,2) - measuresol)/norm(v).integrate(fluid,2));
        std::cout << "Relative solution change: " << convergence << std::endl;

        p.write(fluid, "p.vtk");
        v.write(fluid, "v.vtk", 2);

        index++;
    }

    // Compute the flow velocity norm at position (5,1,0) mm in space:
    double vnorm = norm(v).interpolate(fluid, {5e-3, 1e-3, 0.0})[0];

    // Output the input and output flowrate for a unit width:
    double flowratein = (normal(inlet)*v).integrate(inlet, 4);
    double flowrateout = -(normal(outlet)*v).integrate(outlet, 4);
    std::cout << std::endl << "Flowrate in/out for a unit width: " << flowratein << " / " << flowrateout << " m^3/s" << std::endl;

    // Code validation line. Can be removed.
    std::cout << (vnorm*flowrateout < 2.64489e-05 && vnorm*flowrateout > 2.64485e-05);
}

mesh createmesh(double lthin, double hthin, double lthick, double hthick, int nlthin, int nlthick, int nhthin, int nhthick)
{
    int fluid = 1, inlet = 2, outlet = 3, skin = 4;

    shape qthinleft("quadrangle", fluid, {0,0,0, lthin,0,0, lthin,hthin,0, 0,hthin,0}, {nlthin,nhthin,nlthin,nhthin});
    shape qthinright("quadrangle", fluid, {lthin,0,0, lthin+lthick,0,0, lthin+lthick,hthin,0, lthin,hthin,0}, {nlthick,nhthin,nlthick,nhthin});
    shape qthick("quadrangle", fluid, {lthin,hthin,0, lthin+lthick,hthin,0, lthin+lthick,hthin+hthick,0, lthin,hthin+hthick,0}, {nlthick,nhthick,nlthick,nhthick});

    shape linlet = qthinleft.getsons()[3];
    linlet.setphysicalregion(inlet);
    shape loutlet = shape("union", outlet, {qthick.getsons()[1],qthinright.getsons()[1]});

    mesh mymesh;
    mymesh.regionskin(skin, fluid);
    mymesh.load({qthinleft,qthinright,qthick,linlet,loutlet});

    mymesh.write("channel.msh");

    return mymesh;
}

int main(void)
{	
    SlepcInitialize(0,{},0,0);

    sparselizard();

    SlepcFinalize();

    return 0;
}
