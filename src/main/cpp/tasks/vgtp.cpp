/*
 * Copyright (C) 2012  Brockmann Consult GmbH (info@brockmann-consult.de)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation. This program is distributed in the hope it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "../modules/Col.h"
#include "../modules/Pcl.h"
#include "../modules/Vbm.h"
#include "../modules/Vfl.h"
#include "../modules/Vpr.h"
#include "../reader/SynL1Reader.h"
#include "../writer/VgtWriter.h"
#include "../writer/ManifestWriter.h"

#include "../util/BasicTask.h"

/**
 * The VGT-P processor main routine.
 */
int main(int argc, char* argv[]) {
	BasicTask task("VGTP");

	shared_ptr<Module> reader = shared_ptr<Module>(new SynL1Reader());
	shared_ptr<Module> col = shared_ptr<Module>(new Col());
	shared_ptr<Module> pcl = shared_ptr<Module>(new Pcl());
	shared_ptr<Module> vbm = shared_ptr<Module>(new Vbm());
	shared_ptr<Module> vfl = shared_ptr<Module>(new Vfl());
	shared_ptr<Module> vpr = shared_ptr<Module>(new Vpr());
	shared_ptr<Module> writer = shared_ptr<Module>(new VgtWriter());
	shared_ptr<Module> manifestWriter = shared_ptr<Module>(new ManifestWriter(Constants::PRODUCT_VGP));

	task.getContext().addModule(reader);
	task.getContext().addModule(col);
	task.getContext().addModule(pcl);
	task.getContext().addModule(vbm);
	task.getContext().addModule(vfl);
	task.getContext().addModule(vpr);
	task.getContext().addModule(writer);
	task.getContext().addModule(manifestWriter);

	return task.execute(argc, argv);
}
