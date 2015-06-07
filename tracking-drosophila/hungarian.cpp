/********************************************************************
 ********************************************************************
 **
 ** libhungarian by Cyrill Stachniss, 2004
 **
 **
 ** Solving the Minimum Assignment Problem using the
 ** Hungarian Method.
 **
 ** ** This file may be freely copied and distributed! **
 **
 ** Parts of the used code was originally provided by the
 ** "Stanford GraphGase", but I made changes to this code.
 ** As asked by  the copyright node of the "Stanford GraphGase",
 ** I hereby proclaim that this file are *NOT* part of the
 ** "Stanford GraphGase" distrubition!
 **
 ** This file is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied
 ** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 ** PURPOSE.
 **
 ********************************************************************
 ********************************************************************/


#include "hungarian.hpp"
#include "stdlib.h"

#define INF (0x7FFFFFFF)
#define verbose (0)

#define hungarian_test_alloc(X) do {if ((void *)(X) == NULL) printf("\n ERROR"); } while (0)



CvMat* Hungaro(CvMat* Matrix){

	int p=0;
	float Cost_Matrix[Matrix->rows][Matrix->cols]; // Matriz de Pesos
	CvMat* Matrix_Asignation = NULL;

	for(int x=0;x < Matrix->rows;x++){
		for( int y=0;y < Matrix->cols;y++){
			Cost_Matrix[x][y]=Matrix->data.fl[p];
			p++;
		}
	}

	hungarian_problem_t q;


	q.cost = (float**)calloc( Matrix->cols*Matrix->rows,sizeof(int*));
	for(int k=0;k<Matrix->rows;k++){
		q.cost[k]=(float*)calloc( Matrix->cols,sizeof(int));
		for(int l=0;l<Matrix->cols;l++){
			q.cost[k][l]=Cost_Matrix[k][l];
		}
	}

//	hungarian_print_matrix(q.cost,Matrix->rows ,Matrix->cols );

	int columnas = hungarian_init(&q, Matrix, Matrix->rows, Matrix->cols,HUNGARIAN_MODE_MAXIMIZE_UTIL);

	if( SHOW_MT_DATA ){
	hungarian_print_matrix(q.cost,q.num_rows ,q.num_cols );
	}
//	hungarian_print_matrix(q.assignment,q.num_rows ,q.num_cols );

	hungarian_solve(&q);

	if( SHOW_MT_DATA ){
		hungarian_print_status(&q);
	}
	Matrix_Asignation=cvCreateMat(q.num_rows,q.num_cols,CV_32FC1);

	int v=0;
	for(int i=0;i < q.num_rows;i++){
		for( int j=0;j < q.num_cols;j++){
			Matrix_Asignation->data.fl[v]=q.assignment[i][j];
			v++;
		}
	}

	hungarian_free(&q);

	return Matrix_Asignation;


}

void hungarian_print_matrix(float** C, int rows, int cols) {
  int i,j;
  printf("\n");
  for(i=0; i<rows; i++) {
    printf("\n");
    for(j=0; j<cols; j++) {
      printf("%f ",C[i][j]);
    }
    }
 }

void hungarian_print_assignment(hungarian_problem_t* p) {
  hungarian_print_matrix(p->assignment, p->num_rows, p->num_cols) ;

}

void hungarian_print_costmatrix(hungarian_problem_t* p) {
  hungarian_print_matrix(p->cost, p->num_rows, p->num_cols) ;
}

void hungarian_print_status(hungarian_problem_t* p) {

  printf("\n\n MATRIZ DE COSTES:");
  hungarian_print_costmatrix(p);

  printf("\n\n MATRIZ DE ASIGNACIONES:");
  hungarian_print_assignment(p);

}

int hungarian_imax(int a, int b) {

	return (a<b)?b:a;
}


int hungarian_init(hungarian_problem_t* p, CvMat* cost_m, int rows, int cols, int mode) {

  int org_cols, org_rows;
  float max_cost;
  int ind=0;
  double cost_matrix[cost_m->rows][cost_m->cols];

	for(int x=0;x < cost_m->rows;x++){
		for( int y=0;y < cost_m->cols;y++){
			cost_matrix[x][y]=cost_m->data.fl[ind];
			ind++;
		}
	}

  max_cost = 0;

  org_cols = cols;
  org_rows = rows;

  // is the number of cols  not equal to number of rows ?
  // if yes, expand with 0-cols / 0-cols
  rows = hungarian_imax(cols, rows);
  cols = rows;

  p->num_rows = rows;
  p->num_cols = cols;

  p->cost = (float**)calloc(cols,sizeof(int*));
  p->assignment = (float**)calloc(cols,sizeof(int*));

  for(int i=0; i < p->num_rows; i++) {
    p->cost[i] = (float*)calloc(cols,sizeof(int));
    p->assignment[i] = (float*)calloc(cols,sizeof(int));

    for(int j=0; j<p->num_cols; j++) {
      p->cost[i][j] =  (i < org_rows && j < org_cols) ? cost_matrix[i][j] : 0;
      p->assignment[i][j] = 0;

      if (max_cost < p->cost[i][j])
    	  max_cost = p->cost[i][j];

    }
  }

//  hungarian_print_costmatrix(p);

  if (mode == HUNGARIAN_MODE_MAXIMIZE_UTIL) {
    for(int i=0; i<p->num_rows; i++) {
      for(int j=0; j<p->num_cols; j++) {
        p->cost[i][j] = max_cost - p->cost[i][j];
      }
    }
  }

  return rows;
}




void hungarian_free(hungarian_problem_t* p) {
  int i;
  for(i=0; i<p->num_rows; i++) {
    free(p->cost[i]);
    free(p->assignment[i]);
  }
  free(p->cost);
  free(p->assignment);
  p->cost = NULL;
  p->assignment = NULL;
}

//void hungarian_free(hungarian_problem_t* p) {
//
//  free(p->cost);
//  free(p->assignment);
//  p->cost = NULL;
//  p->assignment = NULL;
//}



void hungarian_solve(hungarian_problem_t* p)
{
  int i, j, m, n, k, l, t,q,unmatched, cost;
  float s;
  int* col_mate;
  int* row_mate;
  int* parent_row;
  int* unchosen_row;
  int* row_dec;
  int* col_inc;
  int* slack;
  int* slack_row;

  cost=0;
  m =p->num_rows;
  n =p->num_cols;

  col_mate = (int*)calloc(p->num_rows,sizeof(int));
  hungarian_test_alloc(col_mate);
  unchosen_row = (int*)calloc(p->num_rows,sizeof(int));
  hungarian_test_alloc(unchosen_row);
  row_dec  = (int*)calloc(p->num_rows,sizeof(int));
  hungarian_test_alloc(row_dec);
  slack_row  = (int*)calloc(p->num_rows,sizeof(int));
  hungarian_test_alloc(slack_row);

  row_mate = (int*)calloc(p->num_cols,sizeof(int));
  hungarian_test_alloc(row_mate);
  parent_row = (int*)calloc(p->num_cols,sizeof(int));
  hungarian_test_alloc(parent_row);
  col_inc = (int*)calloc(p->num_cols,sizeof(int));
  hungarian_test_alloc(col_inc);
  slack = (int*)calloc(p->num_cols,sizeof(int));
  hungarian_test_alloc(slack);

  for (i=0;i<p->num_rows;i++) {
    col_mate[i]=0;
    unchosen_row[i]=0;
    row_dec[i]=0;
    slack_row[i]=0;
  }
  for (j=0;j<p->num_cols;j++) {
    row_mate[j]=0;
    parent_row[j] = 0;
    col_inc[j]=0;
    slack[j]=0;
  }

  for (i=0;i<p->num_rows;++i)
    for (j=0;j<p->num_cols;++j)
      p->assignment[i][j]=HUNGARIAN_NOT_ASSIGNED;

  // Begin subtract column minima in order to start with lots of zeroes 12
  if (verbose)
    printf("\n Using heuristic\n");
  for (l=0;l<n;l++)
    {
      s=p->cost[0][l];
      for (k=1;k<m;k++)
        if (p->cost[k][l]<s)
          s=p->cost[k][l];
      cost+=s;
      if (s!=0)
        for (k=0;k<m;k++)
          p->cost[k][l]-=s;
    }
  // End subtract column minima in order to start with lots of zeroes 12

  // Begin initial state 16
  t=0;
  for (l=0;l<n;l++)
    {
      row_mate[l]= -1;
      parent_row[l]= -1;
      col_inc[l]=0;
      slack[l]=INF;
    }
  for (k=0;k<m;k++)
    {
      s=p->cost[k][0];
      for (l=1;l<n;l++)
        if (p->cost[k][l]<s)
          s=p->cost[k][l];
      row_dec[k]=s;
      for (l=0;l<n;l++)
        if (s==p->cost[k][l] && row_mate[l]<0)
          {
            col_mate[k]=l;
            row_mate[l]=k;
            if (verbose)
              printf("\n matching col %d==row %d\n",l,k);
			  goto row_done;
          }
      col_mate[k]= -1;
      if (verbose)
        printf("\n node %d: unmatched row %d\n",t,k);
      unchosen_row[t++]=k;
      row_done:
      ;
    }
  // End initial state 16

  // Begin Hungarian algorithm 18
  if (t==0)
    goto done;
  unmatched=t;
  while (1)
    {
      if (verbose)
        printf("\n Matched %d rows.\n",m-t);
      q=0;
      while (1)
        {
          while (q<t)
            {
              // Begin explore node q of the forest 19
              {
                k=unchosen_row[q];
                s=row_dec[k];
                for (l=0;l<n;l++)
                  if (slack[l])
                    {
                      int del;
                      del=p->cost[k][l]-s+col_inc[l];
                      if (del<slack[l])
                        {
                          if (del==0)
                            {
                              if (row_mate[l]<0)
                                goto breakthru;
                              slack[l]=0;
                              parent_row[l]=k;
                              if (verbose)
                                printf("\n node %d: row %d==col %d--row %d\n",
                                       t,row_mate[l],l,k);
                              unchosen_row[t++]=row_mate[l];
                            }
                          else
                            {
                              slack[l]=del;
                              slack_row[l]=k;
                            }
                        }
                    }
              }
              // End explore node q of the forest 19
              q++;
            }

          // Begin introduce a new zero into the matrix 21
          s=INF;
          for (l=0;l<n;l++)
            if (slack[l] && slack[l]<s)
              s=slack[l];
          for (q=0;q<t;q++)
            row_dec[unchosen_row[q]]+=s;
          for (l=0;l<n;l++)
            if (slack[l])
              {
                slack[l]-=s;
                if (slack[l]==0)
                  {
                    // Begin look at a new zero 22
                    k=slack_row[l];
                    if (verbose)
                      printf(
                             "\n Decreasing uncovered elements by %f produces zero at [%d,%d]\n",
                             s,k,l);
                    if (row_mate[l]<0)
                      {
                        for (j=l+1;j<n;j++)
                          if (slack[j]==0)
                            col_inc[j]+=s;

                       goto breakthru;
                      }
                    else
                      {
                        parent_row[l]=k;
                        if (verbose)
                          printf("\n node %d: row %d==col %d--row %d\n",t,row_mate[l],l,k);
                        unchosen_row[t++]=row_mate[l];
                      }
                    // End look at a new zero 22
                  }
              }
            else
              col_inc[l]+=s;
          // End introduce a new zero into the matrix 21
        }
      breakthru:;
      // Begin update the matching 20
      if (verbose)
        printf("\n Breakthrough at node %d of %d!\n",q,t);
      while (1)
        {
          j=col_mate[k];
          col_mate[k]=l;
          row_mate[l]=k;
          if (verbose)
            printf("\n rematching col %d==row %d\n",l,k);
          if (j<0)
            break;
          k=parent_row[j];
          l=j;
        }
      // End update the matching 20
      if (--unmatched==0)
        goto done;
      // Begin get ready for another stage 17
      t=0;
      for (l=0;l<n;l++)
        {
          parent_row[l]= -1;
          slack[l]=INF;
        }
      for (k=0;k<m;k++)
        if (col_mate[k]<0)
          {
            if (verbose)
              printf("\n node %d: unmatched row %d\n",t,k);
            unchosen_row[t++]=k;
          }
      // End get ready for another stage 17
    }
 done:

//  // Begin doublecheck the solution 23
//  for (k=0;k<m;k++)
//    for (l=0;l<n;l++)
//      if (p->cost[k][l]<row_dec[k]-col_inc[l])
////    	  cvWaitKey(0);
//        exit(0);
//
//  for (k=0;k<m;k++)
//    {
//      l=col_mate[k];
//      if (l<0 || p->cost[k][l]!=row_dec[k]-col_inc[l]) // cvWaitKey(0);
//        exit(0);
//
//    }
//  k=0;
//  for (l=0;l<n;l++)
//    if (col_inc[l])
//      k++;
//  if (k>m)
//    exit(0);
//  // End doublecheck the solution 23

 // End Hungarian algorithm 18

  for (i=0;i<m;++i)
    {
      p->assignment[i][col_mate[i]]=HUNGARIAN_ASSIGNED;
      /*TRACE("%d - %d\n", i, col_mate[i]);*/
    }
  for (k=0;k<m;++k)
    {
      for (l=0;l<n;++l)
        {
          /*TRACE("%d ",p->cost[k][l]-row_dec[k]+col_inc[l]);*/
          p->cost[k][l]=p->cost[k][l]-row_dec[k]+col_inc[l];
        }
      /*TRACE("\n");*/
    }
  for (i=0;i<m;i++)
    cost+=row_dec[i];
  for (i=0;i<n;i++)
    cost-=col_inc[i];
  if (verbose)
    printf("\n Cost is %d\n",cost);


  free(slack);
  free(col_inc);
  free(parent_row);
  free(row_mate);
  free(slack_row);
  free(row_dec);
  free(unchosen_row);
  free(col_mate);
}
