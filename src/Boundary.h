/*
 * Boundary.h
 *
 * Copyright 2012 Martin Robinson
 *
 * This file is part of RD_3D.
 *
 * RD_3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RD_3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RD_3D.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: 12 Oct 2012
 *      Author: robinsonm
 */

#ifndef BOUNDARY_H_
#define BOUNDARY_H_

#include "Species.h"
#include "Vector.h"
#include "Geometry.h"
#include "Operator.h"
#include "Union.h"
#include "NextSubvolumeMethod.h"
#include <iostream>

namespace Tyche {

template<typename T>
class Boundary: public Operator {
public:
	Boundary<T>(const T& geometry):geometry(geometry) {
	}
	void operator()(const double dt) {
	}

	const T& geometry;
};

template<typename T>
std::ostream& operator<< (std::ostream& out, Boundary<T> &b) {
	return out << "\tBase Boundary at "<< b.geometry;
}


template<typename T>
class DestroyBoundary: public Boundary<T> {
public:
	DestroyBoundary(const T& geometry):
		Boundary<T>(geometry) {}
	void operator()(const double dt);

};

template<typename T>
std::ostream& operator<< (std::ostream& out, DestroyBoundary<T> &b) {
	return out << "\tDestroy Boundary at "<< b.geometry;
}

template<typename T>
class JumpBoundary: public Boundary<T> {
public:
	JumpBoundary(const T& geometry, const Vect3d jump_by):
		Boundary<T>(geometry),jump_by(jump_by) {}
	void operator()(const double dt);
protected:
	const Vect3d jump_by;

};

template<typename T>
std::ostream& operator<< (std::ostream& out, JumpBoundary<T> &b) {
	return out << "\tJump Boundary at "<< b.geometry;
}

template<typename T>
JumpBoundary<T> create_jump_boundary(T& geometry, const Vect3d jump_by) {
	return JumpBoundary<T>(geometry,jump_by);
}


template<typename T>
class DiffusionCorrectedBoundary: public Boundary<T> {
public:
   DiffusionCorrectedBoundary(const T& geometry):
      Boundary<T>(geometry),
      uni(generator,boost::uniform_real<>(0,1)) {

   }
   ~DiffusionCorrectedBoundary() {
	   BOOST_FOREACH(std::vector<double>* i, all_prev_distance) {
		   delete i;
	   }
	   BOOST_FOREACH(std::vector<double>* i, all_curr_distance) {
		   delete i;
	   }
   }
   void add_species(Species &s);

protected:
   void timestep_initialise(const double dt);
   void timestep_finalise();
   bool particle_crossed_boundary(const int p_i, const int s_i);
   void recalc_constants(Species &s, const double new_dt) {
      D_dt = s.D*new_dt;
      test_this_distance_from_wall = 5.0*sqrt(2.0*s.D*new_dt);
   }
   void init_prev_distance(Molecules& mols, std::vector<double>& prev_distance) {
      const int n = mols.size();
      prev_distance.resize(n);
      for (int i = 0; i < n; ++i) {
         prev_distance[i] = this->geometry.distance_to_boundary(mols.r[i]);
      }
   }
   std::vector<std::vector<double>* > all_prev_distance, all_curr_distance;
   double D_dt;
   double test_this_distance_from_wall;
   boost::variate_generator<base_generator_type&, boost::uniform_real<> > uni;
};

template<typename T>
class RemoveBoundaryWithCorrection: public DiffusionCorrectedBoundary<T> {
public:
	RemoveBoundaryWithCorrection(const T& geometry):
		DiffusionCorrectedBoundary<T>(geometry) {}
	void operator()(const double dt);
	void add_species(Species& s);
	Molecules& get_removed(Species& s);
private:
	std::vector<Molecules> removed_molecules;
};

template<typename T>
std::ostream& operator<< (std::ostream& out, RemoveBoundaryWithCorrection<T> &b) {
	return out << "\tRemove Boundary With Correction at "<< b.geometry;
}

template<typename T>
RemoveBoundaryWithCorrection<T> create_remove_boundary_corrected(T& geometry) {
	return RemoveBoundaryWithCorrection<T>(geometry);
}


template<typename T>
class JumpBoundaryWithCorrection: public DiffusionCorrectedBoundary<T> {
public:
	JumpBoundaryWithCorrection(const T& geometry, const Vect3d jump_by):
		DiffusionCorrectedBoundary<T>(geometry),jump_by(jump_by) {}
	void operator()(const double dt);

protected:
   const Vect3d jump_by;

};

template<typename T>
std::ostream& operator<< (std::ostream& out, JumpBoundaryWithCorrection<T> &b) {
	return out << "\tJump Boundary With Correction at "<< b.geometry;
}

template<typename T>
JumpBoundaryWithCorrection<T> create_jump_boundary_corrected(T& geometry, const Vect3d jump_by) {
	return JumpBoundaryWithCorrection<T>(geometry,jump_by);
}


template<typename T>
class ReflectiveBoundary: public Boundary<T> {
public:
	ReflectiveBoundary(const T& geometry):
		Boundary<T>(geometry) {}
	void operator()(const double dt);

};

template<typename T>
std::ostream& operator<< (std::ostream& out, ReflectiveBoundary<T> &b) {
	return out << "\tReflective Boundary at "<< b.geometry;
}

template<typename T>
ReflectiveBoundary<T> create_reflective_boundary(T& geometry) {
	return ReflectiveBoundary<T>(geometry);
}

class FluxBoundary: public Boundary<NullGeometry> {
public:
	FluxBoundary(const Vect3d p, const Vect3d t1, const Vect3d t2, const double rate):
		Boundary<NullGeometry>(NullGeometry()),
		p(p),
		t1(t1),
		t2(t2),
		rate(rate),
		uni1(generator,boost::uniform_real<>(0,t1.norm())),
		uni2(generator,boost::uniform_real<>(0,t2.norm()))
		{}
	void operator()(const double dt);
	const double rate;
	const Vect3d p,t1,t2;
private:
	boost::variate_generator<base_generator_type&, boost::uniform_real<> > uni1, uni2;
};


std::ostream& operator<< (std::ostream& out, FluxBoundary &b);

template<typename T>
class CouplingBoundary_M_to_C: public DiffusionCorrectedBoundary<T> {
public:
	CouplingBoundary_M_to_C(const T& geometry, NextSubvolumeMethod& nsm):
		DiffusionCorrectedBoundary<T>(geometry),nsm(nsm) {}
	void operator()(const double dt);
private:
	NextSubvolumeMethod& nsm;

};

template<typename T>
std::ostream& operator<< (std::ostream& out, CouplingBoundary_M_to_C<T> &b) {
	return out << "\tCoupling Boundary from Molecules to Compartments at "<< std::endl << "\t\t"<< b.geometry;
}

template<typename T>
class CouplingBoundary_C_to_M: public Boundary<T> {
public:
	CouplingBoundary_C_to_M(const T& geometry, NextSubvolumeMethod& nsm):
		Boundary<T>(geometry),nsm(nsm),old_dt(0),
		uni(generator,boost::uniform_real<>(0,1)) {}
	void operator()(const double dt);
	void add_species(Species &s, const double dt);
private:
	NextSubvolumeMethod& nsm;
	double old_dt;
//	std::vector<std::vector<int> > boundary_compartment_indicies;
	//TODO: restricted to axis aligned planes and structured grids!
//	std::vector<std::vector<typename T::SurfaceElementType> > boundary_intersections;
	boost::variate_generator<base_generator_type&, boost::uniform_real<> > uni;
};

template<typename T>
std::ostream& operator<< (std::ostream& out, CouplingBoundary_C_to_M<T> &b) {
	return out << "\tCoupling Boundary from Compartments to Molecules at "<< std::endl << "\t\t"<<b.geometry;
}

}





#include "Boundary.impl.h"



#endif /* BOUNDARY_H_ */