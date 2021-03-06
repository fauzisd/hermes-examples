#define HERMES_REPORT_ALL
#define HERMES_REPORT_FILE "application.log"
#include "definitions.h"

// This example shows how to use the automatic h-adaptivity based on a Kelly-type error estimator for an elliptic
// problem with all types of standard boundary conditions.
//
// PDE: Laplace equation -Laplace u = f, where f = CONST_F.
//
// BC: u = T1 ... fixed temperature on Gamma_left (Dirichlet)
//     du/dn = 0 ... insulated wall on Gamma_inner (Neumann)
//     du/dn = C ... constant heat flux on Gamma_inner (Neumann)
//     du/dn = H*(u - T0) ... heat flux on Gamma_bottom (Newton)
//
// Note that the last BC can be written in the form  du/dn - H*u = -H*T0.
//
// The following parameters can be changed:

const int P_INIT = 2;                             // Initial polynomial degree of all mesh elements.
const int INIT_REF_NUM = 0;                       // Number of initial uniform mesh refinements.
const int CORNER_REF_LEVEL = 0;                  // Number of mesh refinements towards the re-entrant corner.
const double THRESHOLD = 0.3;                     // This is a quantitative parameter of the adapt(...) function and
                                                  // it has different meanings for various adaptive strategies (see below).
const int STRATEGY = 0;                           // Adaptive strategy:
                                                  // STRATEGY = 0 ... refine elements until sqrt(THRESHOLD) times total
                                                  //   error is processed. If more elements have similar errors, refine
                                                  //   all to keep the mesh symmetric.
                                                  // STRATEGY = 1 ... refine all elements whose error is larger
                                                  //   than THRESHOLD times maximum element error.
                                                  // STRATEGY = 2 ... refine all elements whose error is larger
                                                  //   than THRESHOLD.
                                                  // More adaptive strategies can be created in adapt_ortho_h1.cpp.
const int MESH_REGULARITY = -1;                   // Maximum allowed level of hanging nodes:
                                                  // MESH_REGULARITY = -1 ... arbitrary level hangning nodes (default),
                                                  // MESH_REGULARITY = 1 ... at most one-level hanging nodes,
                                                  // MESH_REGULARITY = 2 ... at most two-level hanging nodes, etc.
                                                  // Note that regular meshes are not supported, this is due to
                                                  // their notoriously bad performance.
const double ERR_STOP = 0.1;                      // Stopping criterion for adaptivity (rel. error tolerance between the

const int NDOF_STOP = 60000;                      // Adaptivity process stops when the number of degrees of freedom grows
                                                  // over this limit. This is to prevent h-adaptivity to go on forever.
MatrixSolverType matrix_solver_type = SOLVER_UMFPACK;  // Possibilities: SOLVER_AMESOS, SOLVER_AZTECOO, SOLVER_MUMPS,
                                                  // SOLVER_PETSC, SOLVER_SUPERLU, SOLVER_UMFPACK.


int main(int argc, char* argv[])
{
  // Load the mesh.
  Mesh mesh;
  MeshReaderH2D mloader;
  mloader.load("domain.mesh", &mesh);

  // Perform initial mesh refinements.
  for(int i=0; i < INIT_REF_NUM; i++) mesh.refine_all_elements();
  mesh.refine_towards_vertex(3, CORNER_REF_LEVEL);

  // Initialize boundary conditions
  DefaultEssentialBCConst<double> bc_essential(BDY_LEFT, T1);
  EssentialBCs<double> bcs(&bc_essential);

  // Create an H1 space with default shapeset.
  H1Space<double> space(&mesh, &bcs, P_INIT);
  int ndof = Space<double>::get_num_dofs(&space);
  info("ndof = %d", ndof);

  // Initialize the weak formulation.
  WeakFormPoisson wf(EPS_1, EPS_2, H, T0, MATERIAL_1, MATERIAL_2, BDY_BOTTOM, BDY_OUTER, CONST_GAMMA_OUTER);

  // Initialize coarse and reference mesh solution.
  Solution<double> sln;
  
  // Initialize views.
  ScalarView sview("Solution", new WinGeom(0, 0, 410, 600));
  sview.fix_scale_width(50);
  sview.show_mesh(false);
  OrderView  oview("Polynomial orders", new WinGeom(420, 0, 400, 600));

  // DOF and CPU convergence graphs initialization.
  SimpleGraph graph_dof, graph_cpu;
  
  // Time measurement.
  TimePeriod cpu_time;
  cpu_time.tick();
  
  // Initialize matrix solver.
  SparseMatrix<double>* matrix = create_matrix<double>(matrix_solver_type);
  Vector<double>* rhs = create_vector<double>(matrix_solver_type);
  LinearSolver<double>* solver = create_linear_solver<double>(matrix_solver_type, matrix, rhs);
  
  // Adaptivity loop:
  int as = 1; 
  bool done = false;
  do
  {
    info("---- Adaptivity step %d:", as);
       
    // Assemble reference problem.
    info("Solving on reference mesh.");
    DiscreteProblem<double>* dp = new DiscreteProblem<double>(&wf, &space);
    dp->assemble(matrix, rhs);
    
    // Time measurement.
    cpu_time.tick();
    
    // Solve the linear system of the reference problem. 
    // If successful, obtain the solution.
    if(solver->solve()) Solution<double>::vector_to_solution(solver->get_sln_vector(), &space, &sln);
    else error ("Matrix solver failed.\n");
    
    // Time measurement.
    cpu_time.tick();
    
    // View the coarse mesh solution and polynomial orders.
    sview.show(&sln);
    oview.show(&space);
    
    // Skip visualization time.
    cpu_time.tick(HERMES_SKIP);
    
    // Calculate element errors and total error estimate.
    info("Calculating error estimate."); 

    // FIXME
    Hermes::vector<Space<double>*> spaces;
    spaces.push_back(&space);

    KellyTypeAdapt<double>* adaptivity = new KellyTypeAdapt<double>(spaces);
    adaptivity->add_error_estimator_surf(new ErrorEstimatorFormInterface(0));
    adaptivity->add_error_estimator_surf(new ErrorEstimatorFormNewton(0, BDY_BOTTOM));
    adaptivity->add_error_estimator_surf(new ErrorEstimatorFormNeumann(0, BDY_OUTER));
    adaptivity->add_error_estimator_surf(new ErrorEstimatorFormZeroNeumann(0, BDY_INNER));

    double err_est_rel = adaptivity->calc_err_est(&sln, HERMES_TOTAL_ERROR_REL | HERMES_ELEMENT_ERROR_REL) * 100;
                                                  
    // Report results.
    info("ndof: %d, err_est_rel: %g%%", Space<double>::get_num_dofs(&space), err_est_rel);
    
    // Time measurement.
    cpu_time.tick();
    
    // Add entry to DOF and CPU convergence graphs.
    graph_dof.add_values(Space<double>::get_num_dofs(&space), err_est_rel);
    graph_dof.save("conv_dof_est.dat");
    graph_cpu.add_values(cpu_time.accumulated(), err_est_rel);
    graph_cpu.save("conv_cpu_est.dat");
    
    // If err_est too large, adapt the mesh.
    if (err_est_rel < ERR_STOP) done = true;
    else 
    {
      info("Adapting coarse mesh.");
      done = adaptivity->adapt(THRESHOLD, STRATEGY, MESH_REGULARITY);
    }
    if (Space<double>::get_num_dofs(&space) >= NDOF_STOP) done = true;
    
    // Increase the counter of performed adaptivity steps.
    if (done == false)  as++;
    
    // Clean up.
    delete adaptivity;
    delete dp;                                    
  }
  while (done == false);
  
  // Clean up.
  delete solver;
  delete matrix;
  delete rhs;
  
  verbose("Total running time: %g s", cpu_time.accumulated());
  
  // Wait for all views to be closed.
  View::wait();
  
  return 0;
}
