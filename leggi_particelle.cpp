#ifndef __LEGGI_PARTICELLE_C
#define __LEGGI_PARTICELLE_C

#include "leggi_particelle.h"



int leggi_particelle(char* fileIN, parametri binning)
{
	int out_swap = binning.p[SWAP];
	int out_binary = binning.p[OUT_BINARY];
	int out_ascii = binning.p[OUT_ASCII];
	int out_parameters = binning.p[OUT_PARAMS];
	int fai_slices = binning.p[DO_BINNING];
	int cerca_minmax = binning.p[FIND_MINMAX];

	float whichbinx = 0., whichbinpx = 0.;
	int whichbinx_int = (int) whichbinx;
	int whichbinpx_int = (int) whichbinpx;
	float whichbinE = 0., whichbintheta = 0.;
	int whichbinE_int = (int) whichbinE;
	int whichbintheta_int = (int) whichbintheta;

	int ipc,N_param, *int_param,npart_loc;
	int buff, pID;

	int nelab=3;	//3 valori per ora: gamma, theta ed energia
	int conta_nptot = 0;
	float x,y,z,px,py,pz;
	float *estremi_min, *estremi_max;

	short buffshort[2];
	float *particelle, *real_param;
	//	float *particelle_elaborate;
	float gamma, theta, E;
	char nomefile_binary[MAX_LENGTH_FILENAME];
	char nomefile_ascii[MAX_LENGTH_FILENAME];
	char nomefile_parametri[MAX_LENGTH_FILENAME];
	std::ostringstream nomefile_xpx, nomefile_Etheta, nomefile_Espec, nomefile_Estremi;

	FILE *file_in;
	file_in=fopen(fileIN, "r");
	FILE *binary_all_out;
	FILE *ascii_all_out;
	FILE *parameters;
	std::ofstream xpx_out, Etheta_out, Espec_out, Estremi_out;
	size_t fread_size;

	int npe,nx,ny,nz,ibx,iby,ibz,model,dmodel,nsp,ndim,lpord,deord,nptot, ny_loc, np_loc,ndv;
	float tnow,xmin,xmax,ymin,ymax,zmin,zmax,w0x,w0y,nrat,a0,lam0,E0,ompe,xt_in,xt_end,charge,mass, np_over_nm;
	float rx, ry, rz, ux, uy, uz, wgh;
	int tipo = 0;
	if (fileIN[0] == 'E') tipo = 3;
	else if (fileIN[0] == 'P') tipo = 1;
	else printf("Tipo non riconosciuto!\n");

	fread_size = std::fread((void*) &buff,sizeof(int),1,file_in);
	fread_size = std::fread((void*) &N_param,sizeof(int),1,file_in);
	fread_size = std::fread((void*) &buff,sizeof(int),1,file_in);
	if (out_swap) swap_endian_i(&N_param,1);
	printf("numero parametri=%i\n",N_param);
	fflush(stdout);
	int_param=(int*)malloc(N_param*sizeof(int));
	real_param=(float*)malloc(N_param*sizeof(float));
	fread_size = std::fread(&buff,sizeof(int),1,file_in);
	fread_size = std::fread(int_param,sizeof(int),N_param,file_in);
	fread_size = std::fread(&buff,sizeof(int),1,file_in);
	fread_size = std::fread(&buff,sizeof(int),1,file_in);
	fread_size = std::fread(real_param,sizeof(float),N_param,file_in);
	fread_size = std::fread(&buff,sizeof(int),1,file_in);
	if (out_swap) swap_endian_i(int_param,N_param);
	if (out_swap) swap_endian_f(real_param,N_param);
	npe=int_param[0];     //numero processori
	nx=int_param[1];
	ny_loc=int_param[2];
	nz=int_param[3];
	ibx=int_param[4];
	iby=int_param[5];
	ibz=int_param[6];
	model=int_param[7];  //modello di laser utilizzato
	dmodel=int_param[8]; //modello di condizioni iniziali
	nsp=int_param[9];    //numero di speci
	ndim=int_param[10];  //numero di componenti dello spazio dei momenti

	estremi_min = new float[2*ndim+nelab];
	estremi_max = new float[2*ndim+nelab];
	for (int i = 0; i < (2*ndim+nelab); i++)
	{
		estremi_min[i] = (float) NUMERO_MASSIMO;
		estremi_max[i] = (float) -NUMERO_MASSIMO;
	}

	np_loc=int_param[11];  //
	lpord=int_param[12]; //ordine dello schema leapfrog
	deord=int_param[13]; //ordine derivate
	//current type
	pID=int_param[15];  //tipo delle particelle
	nptot=int_param[16]; //numero di particelle da leggere nella specie
	ndv=int_param[17]; //numero di particelle da leggere nella specie
	tnow=real_param[0];  /*tempo dell'output*/
	xmin=real_param[1];  //estremi della griglia
	xmax=real_param[2];  //estremi della griglia
	ymin=real_param[3];  //estremi della griglia
	ymax=real_param[4];  //estremi della griglia
	zmin=real_param[5];  //estremi della griglia
	zmax=real_param[6];  //estremi della griglia
	w0x=real_param[7];      //waist del laser in x
	w0y=real_param[8];      //waist del laser in y
	nrat=real_param[9];     //n over n critical
	a0=real_param[10];      // a0 laser
	lam0=real_param[11];    // lambda
	E0=real_param[12];      //conversione da campi numerici a TV/m
	ompe=real_param[13];    //costante accoppiamento correnti campi
	np_over_nm=real_param[14];		//numerical2physical particles 14 
	xt_in=real_param[15];			//inizio plasma
	xt_end=real_param[16];
	charge=real_param[17];			//carica particella su carica elettrone
	mass=real_param[18];			//massa particelle su massa elettrone
	ny=ny_loc*npe;
	printf("nptot=%i\n",int_param[16]); 
	printf("\ninizio processori \n");
	fflush(stdout);
	float **xpx = new float* [binning.nbin_x+3];
	for (int i = 0; i < binning.nbin_x+3; i++)
	{
		xpx[i] = new float [binning.nbin_px+3];
		for (int j = 0; j < binning.nbin_px+3; j++) xpx[i][j] = 0.0;
	}
	float **Etheta = new float* [binning.nbin_E+3];
	for (int i = 0; i < binning.nbin_E+3; i++)
	{
		Etheta[i] = new float [binning.nbin_theta+3];
		for (int j = 0; j < binning.nbin_theta+3; j++) Etheta[i][j] = 0.0;
	}
	float *Espec = new float [binning.nbin_E+3];

	sprintf(nomefile_ascii,"%s.ascii",fileIN);
	sprintf(nomefile_binary,"%s.clean",fileIN);

	for(ipc=0;ipc<npe;ipc++)
	{
		fread_size = std::fread(&buff,sizeof(int),1,file_in); 
		fread_size = std::fread(&npart_loc,sizeof(int),1,file_in);
		fread_size = std::fread(&buff,sizeof(int),1,file_in);
		if (out_swap) swap_endian_i(&npart_loc,1);
		particelle=(float*)malloc(npart_loc*(2*ndim+WEIGHT)*sizeof(float));
		//		particelle_elaborate=(float*)malloc(npart_loc*nelab*sizeof(float));
		printf("proc number %i\tnpart=%i\n",ipc,npart_loc);
		if(npart_loc>0)
		{
			printf("\tentro\t");
			fflush(stdout);
			fread_size = std::fread(buffshort,sizeof(short),2,file_in);
			if (out_swap) swap_endian_s(buffshort,2);
			printf("lunghezza=%i    %hu\t%hu\n",npart_loc*2*ndim,buffshort[0],buffshort[1]);
			fread_size = std::fread(particelle,sizeof(float),npart_loc*(2*ndim+WEIGHT),file_in);
			fread_size = std::fread(&buff,sizeof(int),1,file_in);
			if (out_swap) swap_endian_f(particelle,npart_loc*(2*ndim+WEIGHT));

			if (cerca_minmax && ndim == 3 && !fai_slices)
			{
				for (int i = 0; i < npart_loc; i++)
				{
					//				x=fabs(*(particelle+i*(2*ndim+WEIGHT)));
					//				y=fabs(*(particelle+i*(2*ndim+WEIGHT)+1));
					//				z=fabs(*(particelle+i*(2*ndim+WEIGHT)+2));
					//				px=fabs(*(particelle+i*(2*ndim+WEIGHT)+3));
					//				py=fabs(*(particelle+i*(2*ndim+WEIGHT)+4));
					//				pz=fabs(*(particelle+i*(2*ndim+WEIGHT)+5));
					x=*(particelle+i*(2*ndim+WEIGHT));
					y=*(particelle+i*(2*ndim+WEIGHT)+1);
					z=*(particelle+i*(2*ndim+WEIGHT)+2);
					px=*(particelle+i*(2*ndim+WEIGHT)+3);
					py=*(particelle+i*(2*ndim+WEIGHT)+4);
					pz=*(particelle+i*(2*ndim+WEIGHT)+5);
					gamma=(float)(sqrt(1.+px*px+py*py+pz*pz)-1.);			//gamma
					theta=(float)(atan2(sqrt(py*py+pz*pz),px)*180./M_PI);	//theta nb: py e pz sono quelli trasversi in ALaDyn!
					E=(float)(gamma*P_MASS);								//energia
					if (x < estremi_min[0]) estremi_min[0] = x;
					if (x > estremi_max[0]) estremi_max[0] = x;
					if (y < estremi_min[1]) estremi_min[1] = y;
					if (y > estremi_max[1]) estremi_max[1] = y;
					if (z < estremi_min[2]) estremi_min[2] = z;
					if (z > estremi_max[2]) estremi_max[2] = z;
					if (px < estremi_min[3]) estremi_min[3] = px;
					if (px > estremi_max[3]) estremi_max[3] = px;
					if (py < estremi_min[4]) estremi_min[4] = py;
					if (py > estremi_max[4]) estremi_max[4] = py;
					if (pz < estremi_min[5]) estremi_min[5] = pz;
					if (pz > estremi_max[5]) estremi_max[5] = pz;
					if (gamma < estremi_min[6]) estremi_min[6] = gamma;
					if (gamma > estremi_max[6]) estremi_max[6] = gamma;
					if (theta < estremi_min[7]) estremi_min[7] = theta;
					if (theta > estremi_max[7]) estremi_max[7] = theta;
					if (E < estremi_min[8]) estremi_min[8] = E;
					if (E > estremi_max[8]) estremi_max[8] = E;
				}

			}


			if (fai_slices && ndim == 3 && !cerca_minmax)
			{
				for (int i = 0; i < npart_loc; i++)
				{
					x=*(particelle+i*(2*ndim+WEIGHT));
					y=*(particelle+i*(2*ndim+WEIGHT)+1);
					z=*(particelle+i*(2*ndim+WEIGHT)+2);
					px=*(particelle+i*(2*ndim+WEIGHT)+3);
					py=*(particelle+i*(2*ndim+WEIGHT)+4);
					pz=*(particelle+i*(2*ndim+WEIGHT)+5);
					//				particelle_elaborate[i]=(float)(sqrt(1.+px*px+py*py+pz*pz)-1.);				//gamma
					//				particelle_elaborate[i+1]=(float)(atan2(sqrt(py*py+pz*pz),px)*180./M_PI);	//theta nb: py e pz sono quelli trasversi in ALaDyn!
					//				particelle_elaborate[i+2]=(float)(particelle_elaborate[i]*P_MASS);			//energia
					gamma=(float)(sqrt(1.+px*px+py*py+pz*pz)-1.);			//gamma
					theta=(float)(atan2(sqrt(py*py+pz*pz),px)*180./M_PI);	//theta nb: py e pz sono quelli trasversi in ALaDyn!
					E=(float)(gamma*P_MASS);								//energia

					// xpx
					if (x < binning.xmin)
					{
						//					whichbinx = 0.0;
						whichbinx_int = 0;
					}
					else if (x > binning.xmax)
					{
						//					whichbinx = (float) (binning.nbin_x + 2);
						whichbinx_int = binning.nbin_x + 2;
					}
					else
					{
						whichbinx = (x - binning.xmin) / binning.dimmi_dimx();
						whichbinx_int = (int)(whichbinx+1.0);
					}
					if (px < binning.pxmin)
					{
						//					whichbinpx = 0.0;
						whichbinpx_int = 0;
					}
					else if (px > binning.pxmax)
					{
						//					whichbinpx = (float) (binning.nbin_px + 2);
						whichbinpx_int = binning.nbin_px + 2;
					}
					else
					{
						whichbinpx = (px - binning.pxmin) / binning.dimmi_dimpx();
						whichbinpx_int = (int)(whichbinpx+1.0);
					}
					if (WEIGHT) xpx[whichbinx_int][whichbinpx_int] += *(particelle+i*(2*ndim+WEIGHT)+6);
					else		xpx[whichbinx_int][whichbinpx_int] += 1.0;

					// Etheta
					if (E < binning.Emin)
					{
						//					whichbinE = 0.0;
						whichbinE_int = 0;
					}
					else if (E > binning.Emax)
					{
						//					whichbinE = (float) (binning.nbin_E + 2);
						whichbinE_int = binning.nbin_E + 2;
					}
					else
					{
						whichbinE = (E - binning.Emin) / binning.dimmi_dimE();
						whichbinE_int = (int)(whichbinE+1.0);
					}
					if (theta < binning.thetamin)
					{
						//					whichbintheta = 0.0;
						whichbintheta_int = 0;
					}
					else if (theta > binning.thetamax)
					{
						//					whichbintheta = (float) (binning.nbin_theta + 2);
						whichbintheta_int = binning.nbin_theta + 2;
					}
					else
					{
						whichbintheta = (theta - binning.thetamin) / binning.dimmi_dimtheta();
						whichbintheta_int = (int)(whichbintheta+1.0);
					}
					if (WEIGHT) Etheta[whichbinE_int][whichbintheta_int] += *(particelle+i*(2*ndim+WEIGHT)+6);
					else		Etheta[whichbinE_int][whichbintheta_int] += 1.0;

					// Espec
					if (WEIGHT) Espec[whichbinE_int] += *(particelle+i*(2*ndim+WEIGHT)+6);
					else		Espec[whichbinE_int] += 1.0;

				}
			}

			if (out_binary)
			{
				binary_all_out=fopen(nomefile_binary, "ab");
				printf("\nWriting the C binary file\n");
				fwrite((void*)particelle,sizeof(float),nptot*(2*ndim+WEIGHT),binary_all_out);
				fflush(binary_all_out);
				fclose(binary_all_out);
			}




			if(out_ascii)
			{
				ascii_all_out=fopen(nomefile_ascii, "a");

				for(int i=0;i<nptot;i++)
				{
					rx=particelle[i*(6+WEIGHT)+1]*((float)1.e-4);
					ry=particelle[i*(6+WEIGHT)+2]*((float)1.e-4);
					rz=particelle[i*(6+WEIGHT)+0]*((float)1.e-4);
					ux=particelle[i*(6+WEIGHT)+4];
					uy=particelle[i*(6+WEIGHT)+5];
					uz=particelle[i*(6+WEIGHT)+3];
					if (WEIGHT)
					{
						wgh=particelle[i*(6+WEIGHT)+6];
						fprintf(ascii_all_out,"%e %e %e %e %e %e %d %e 0 %d\n",rx, ry, rz, ux, uy, uz, tipo, wgh, i+1);
					}
					else
					{
						fprintf(ascii_all_out,"%e %e %e %e %e %e %d 1 0 %d\n",rx, ry, rz, ux, uy, uz, tipo, i+1);
					}
				}
				fflush(ascii_all_out);
				fclose(ascii_all_out);
			}

			free(particelle);
			//		free(particelle_elaborate);
		}
	}


	if (out_parameters)
	{
		sprintf(nomefile_parametri,"%s.parameters",fileIN);
		parameters=fopen(nomefile_parametri, "w");
		printf("\nWriting the parameters file\n");
		fprintf(parameters,"interi\n");
		fprintf(parameters,"npe=%i\n",int_param[0]);     //numero processori
		fprintf(parameters,"nx=%i\n",int_param[1]);
		fprintf(parameters,"ny=%i\n",int_param[2]);
		fprintf(parameters,"nz=%i\n",int_param[3]);
		fprintf(parameters,"ibx=%i\n",int_param[4]);
		fprintf(parameters,"iby=%i\n",int_param[5]);
		fprintf(parameters,"ibz=%i\n",int_param[6]);
		fprintf(parameters,"model=%i\n",int_param[7]);  //modello di laser utilizzato
		fprintf(parameters,"dmodel=%i\n",int_param[8]); //modello di condizioni iniziali
		fprintf(parameters,"nsp=%i\n",int_param[9]);    //numero di speci
		fprintf(parameters,"ndim=%i\n",int_param[10]);   
		fprintf(parameters,"np_loc=%i\n",int_param[11]);  
		fprintf(parameters,"lpord=%i\n",int_param[12]); //ordine dello schema leapfrog
		fprintf(parameters,"deord=%i\n",int_param[13]); //ordine derivate
		fprintf(parameters,"pID=%i\n",int_param[15]); 
		fprintf(parameters,"nptot=%i\n",int_param[16]); 
		fprintf(parameters,"ndv=%i\n",int_param[17]); 
		fprintf(parameters,"========= fine interi\n");
		fprintf(parameters,"\n floating\n");
		fprintf(parameters,"tnow=%f\n",real_param[0]);  //tempo dell'output
		fprintf(parameters,"xmin=%f\n",real_param[1]);  //estremi della griglia
		fprintf(parameters,"xmax=%f\n",real_param[2]);  //estremi della griglia
		fprintf(parameters,"ymin=%f\n",real_param[3]);  //estremi della griglia
		fprintf(parameters,"ymax=%f\n",real_param[4]);  //estremi della griglia
		fprintf(parameters,"zmin=%f\n",real_param[5]);  //estremi della griglia
		fprintf(parameters,"zmax=%f\n",real_param[6]);  //estremi della griglia
		fprintf(parameters,"w0x=%f\n",real_param[7]);      //waist del laser in x
		fprintf(parameters,"w0y=%f\n",real_param[8]);      //waist del laser in y
		fprintf(parameters,"nrat=%f\n",real_param[9]);     //n over n critical
		fprintf(parameters,"a0=%f\n",real_param[10]);      // a0 laser
		fprintf(parameters,"lam0=%f\n",real_param[11]);    // lambda
		fprintf(parameters,"E0=%f\n",real_param[12]);      //conversione da campi numerici a TV/m
		fprintf(parameters,"ompe=%f\n",real_param[13]);    //costante accoppiamento correnti campi
		fprintf(parameters,"np_over_nm=%f\n",real_param[14]);   //numerical2physical particles 14 
		fprintf(parameters,"xt_in=%f\n",real_param[15]);
		fprintf(parameters,"xt_end=%f\n",real_param[16]);
		fprintf(parameters,"charge=%f\n",real_param[17]);  //carica particella su carica elettrone
		fprintf(parameters,"mass=%f\n",real_param[18]);    //massa particelle su massa elettrone
		//		if(WEIGHT) fprintf(parameters,"weight=%f\n",particelle[6]);    //massa particelle su massa elettrone
		fclose(parameters);
	}

	if (!fai_slices && cerca_minmax && ndim == 3)
	{
		nomefile_Estremi << fileIN << "_extremes";
		Estremi_out.open(nomefile_Estremi.str().c_str());
		Estremi_out << "XMIN = " << estremi_min[0] << std::endl;
		Estremi_out << "XMAX = " << estremi_max[0] << std::endl;
		Estremi_out << "YMIN = " << estremi_min[1] << std::endl;
		Estremi_out << "YMAX = " << estremi_max[1] << std::endl;
		Estremi_out << "ZMIN = " << estremi_min[2] << std::endl;
		Estremi_out << "ZMAX = " << estremi_max[2] << std::endl;
		Estremi_out << "PXMIN = " << estremi_min[3] << std::endl;
		Estremi_out << "PXMAX = " << estremi_max[3] << std::endl;
		Estremi_out << "PYMIN = " << estremi_min[4] << std::endl;
		Estremi_out << "PYMAX = " << estremi_max[4] << std::endl;
		Estremi_out << "PZMIN = " << estremi_min[5] << std::endl;
		Estremi_out << "PZMAX = " << estremi_max[5] << std::endl;
		Estremi_out << "GAMMAMIN = " << estremi_min[6] << std::endl;
		Estremi_out << "GAMMAMAX = " << estremi_max[6] << std::endl;
		Estremi_out << "THETAMIN = " << estremi_min[7] << std::endl;
		Estremi_out << "THETAMAX = " << estremi_max[7] << std::endl;
		Estremi_out << "EMIN = " << estremi_min[8] << std::endl;
		Estremi_out << "EMAX = " << estremi_max[8] << std::endl;
		Estremi_out.close();
	}


	if (ndim == 3 && fai_slices && !cerca_minmax)
	{
		nomefile_xpx << fileIN << "_xpx";
		nomefile_Etheta << fileIN << "_Etheta";
		nomefile_Espec << fileIN << "_Espec";
		xpx_out.open(nomefile_xpx.str().c_str());
		Etheta_out.open(nomefile_Etheta.str().c_str());
		Espec_out.open(nomefile_Espec.str().c_str());

		float min1, min2, max1, max2;

		min1=binning.Emin-binning.dimmi_dimE();
		max1=binning.Emin;
		for (int i = 0; i < binning.nbin_E+3; i++)
		{
			Espec_out << std::setprecision(7) << min1 << "\t" << max1 << "\t" << Espec[i] << std::endl;

			min1 += binning.dimmi_dimE();
			max1 += binning.dimmi_dimE();
		}


		min1=binning.Emin-binning.dimmi_dimE();
		max1=binning.Emin;
		min2=binning.thetamin-binning.dimmi_dimtheta();
		max2=binning.thetamin;

		for (int i = 0; i < binning.nbin_E+3; i++)
		{
			for (int j = 0; j < binning.nbin_theta+3; j++)
			{
				Etheta_out << std::setprecision(7) << min1 << "\t" << max1 << "\t" << min2 << "\t" << max2 << "\t" << Etheta[i][j] << std::endl;
				min2 += binning.dimmi_dimtheta();
				max2 += binning.dimmi_dimtheta();
			}
			min1 += binning.dimmi_dimE();
			max1 += binning.dimmi_dimE();
			min2 = binning.thetamin-binning.dimmi_dimtheta();
			max2 = binning.thetamin;

		}


		min1=binning.xmin-binning.dimmi_dimx();
		max1=binning.xmin;
		min2=binning.pxmin-binning.dimmi_dimpx();
		max2=binning.pxmin;

		for (int i = 0; i < binning.nbin_x+3; i++)
		{
			for (int j = 0; j < binning.nbin_px+3; j++)
			{
				xpx_out << std::setprecision(7) << min1 << "\t" << max1 << "\t" << min2 << "\t" << max2 << "\t" << xpx[i][j] << std::endl;
				min2 += binning.dimmi_dimpx();
				max2 += binning.dimmi_dimpx();
			}
			min1 += binning.dimmi_dimx();
			max1 += binning.dimmi_dimx();
			min2 = binning.pxmin-binning.dimmi_dimpx();
			max2 = binning.pxmin;

		}

		xpx_out.close();
		Etheta_out.close();
		Espec_out.close();

	}

	return 0;
}


#endif