#include <stdio.h>
#include <math.h>

#include "dy_ustack.h"
#include "dy_math.h"
#include <time.h>

#define ASSERT(cond) if(!(cond)) {printf("Assertion failed!\n%d: " __FUNCTION__ "\n\t" #cond "\n", __LINE__); return 1;}


int test_dy_ustack()
{
	for (int z = 1; z < 71; z++)
	{
		dy_ustack<int> stack;

		for (int k = 1; k < 71; k++)
		{
			int max = k * z;

			for (int i = 0; i < max; i++)
				stack.push(&i);

			ASSERT(stack.count == max);
			ASSERT(*stack.first() == 0);
			ASSERT(*stack.last() == max - 1);

			for (int i = 0; i < max; i++)
			{
				int* ele = stack.seek(i);
				ASSERT(*ele == i);
			}

			int m = 0;
			for (auto ele : stack)
			{
				ASSERT(*ele == m);
				m++;
			}
			for (int i = 0; i < max; i++)
				stack.pop();
			ASSERT(stack.count == 0);
			ASSERT(stack.front == 0);
			ASSERT(stack.back == 0);
		}
	}

	return 0;
}

bool solve_cramer(mat3 const& A, vec3 const& b, vec3* out)
{

	mat3 t;
	vec3 x;
	float d = A.det();
	if (d == 0) return false;

	t = A;
	t.a = b;
	x.x = t.det() / d;

	t = A;
	t.b = b;
	x.y = t.det() / d;

	t = A;
	t.c = b;
	x.z = t.det() / d;

	*out = x;
	return true;
}

// Somewhat of a statistical regression test
// If we're doing worse on average when compared to cramer, we've regressed
int test_dy_mat3_solve()
{

	srand((unsigned)time(0));

	int total = 51200;
	
	int no_solve = 0;
	int equal = 0;

	int xtally = 0;
	int ytally = 0;
	
	float xtotalerr = 0;
	float ytotalerr = 0;

	float xmaxerr = 0;
	float ymaxerr = 0;

	float xstavg  = 0;
	float ystavg  = 0;
	float xmedian = 0;
	float ymedian = 0;

	int valid = 0;

	for (int i = 0; i < total; i++)
	{
		mat3 m;
		vec3* mf = reinterpret_cast<vec3*>(&m);
		vec3 ans;

		int id = i % 5;
		if (id == 0)
		{
			// Totally random small numbers
			for (int k = 0; k < 3; k++)
				mf[k] = { (rand() % 800 - 400) / 40.0f, (rand() % 800 - 400) / 40.0f, (rand() % 800 - 400) / 40.0f };

			ans = { (rand() % 800 - 400) / 40.0f, (rand() % 800 - 400) / 40.0f, (rand() % 800 - 400) / 40.0f };
		}
		else if (id == 1)
		{
			// Totally random very large numbers with a small input
			for (int k = 0; k < 3; k++)
				mf[k] = { rand() % 40000 - 20000.0f, rand() % 40000 - 20000.0f, rand() % 40000 - 20000.0f };

			ans = { (rand() % 800 - 400) / 400.0f, (rand() % 800 - 400) / 400.0f, (rand() % 800 - 400) / 400.0f };
		}
		else if (id == 2)
		{
			// Totally random very small numbers with a large input
			for (int k = 0; k < 3; k++)
				mf[k] = { (rand() % 800 - 400) / 400.0f, (rand() % 800 - 400) / 400.0f, (rand() % 800 - 400) / 400.0f };


			ans = { rand() % 40000 - 20000.0f, rand() % 40000 - 20000.0f, rand() % 40000 - 20000.0f };
		}
		else if(id == 3)
		{
			// Totally random large whole numbers
			for (int k = 0; k < 3; k++)
				mf[k] = { rand() % 40000 - 20000.0f, rand() % 40000 - 20000.0f, rand() % 40000 - 20000.0f};

			ans = { rand() % 40000 - 20000.0f, rand() % 40000 - 20000.0f, rand() % 40000 - 20000.0f};
		}
		else
		{
			// Create test data using polar coords

			for (int k = 0; k < 3; k++)
			{
				float a = (rand() % 720 - 360) / 360.0f * DY_PI * 2;
				float b = (rand() % 720 - 360) / 360.0f * DY_PI * 0.45;

				mf[k] = { cosf(b) * cosf(a), sinf(b), cosf(b) * sinf(a) };
			}

			ans =
			{
				(rand() % 16000 - 8000) * cosf(rand() % 360 / 360.0f * DY_PI) + rand() % 4 - 2,
				(rand() % 16000 - 8000) * cosf(rand() % 360 / 360.0f * DY_PI) + rand() % 4 - 2,
				(rand() % 16000 - 8000) * cosf(rand() % 360 / 360.0f * DY_PI) + rand() % 4 - 2
			};
		}

		vec3 xans = ans;
		vec3 yans = ans;

		// This second for loop is for investigating drift
		// TODO: Use this for metrics
		for (int k = 0; k < 900; k++)
		{
			vec3 xb = m * xans;
			vec3 yb = m * yans;

			vec3 x;
			bool X = mat3::solve(m, xb, &x);

			vec3 y;
			bool Y = solve_cramer(m, yb, &y);


			float xerr = 0;
			if (X)
				xerr = (ans - x).mag() / ans.mag();

			float yerr = 0;
			if (Y)
				yerr = (ans - y).mag() / ans.mag();

			//printf("%.03f% | %.03f%\n", xerr * 100, yerr * 100);

			if (!X || !Y)
				no_solve++;
			else if (xerr < yerr)
				xtally++;
			else if (xerr > yerr)
				ytally++;
			else
				equal++;

			if (X == Y && X)
			{
				xtotalerr += xerr;
				ytotalerr += yerr;

				if (xerr > xmaxerr) xmaxerr = xerr;
				if (yerr > ymaxerr) ymaxerr = yerr;

				xstavg = (xstavg * 0.85f + xerr * 0.15f);
				ystavg = (ystavg * 0.85f + yerr * 0.15f);

				xmedian += xstavg;
				ymedian += ystavg;

				valid++;
			}

			if (X && !Y)
			{
				if (xerr > 2)
					printf("_____! Failure to detect %f!\n", xerr);
				//			ASSERT(xerr < 2);
			}

			xans = x;
			yans = y;
		}
	}
	float xavgerr = xtotalerr / valid;
	float yavgerr = ytotalerr / valid;

	xmedian /= valid;
	ymedian /= valid;

	printf("Results for %d equations\n", total);
	printf("Wins: %f | %f\n", xtally / (float)(valid - equal), ytally / (float)(valid - equal));
	printf("Average error: %.04f | %.04f\n", xavgerr * 100, yavgerr * 100);
	printf("Median-ish: %.04f | %.04f\n", xmedian * 100, ymedian * 100);
	printf("Worst error: %.04f | %.04f\n", xmaxerr * 100, ymaxerr * 100);
	printf("Win ratio x/y: %f\n", xtally / (float)ytally);
	printf("Avg error ratio y/x: %.04f\n", yavgerr / xavgerr);
	printf("Median error ratio y/x: %.04f\n", ymedian / xmedian);
	printf("Worst error ratio y/x: %.04f\n", ymaxerr / xmaxerr);
	printf("Could not solve %d\n", no_solve);
	printf("Same result %d\n", equal);
	//printf("%.03f\n", (xtally - ytally) / (float)(valid) * 100);
	//printf("%.03f\n", yavgerr / xavgerr * 100);

	ASSERT(xtally > ytally);
	ASSERT(xavgerr < yavgerr);
	return 0;
}

struct dy_test
{
	const char* name;
	int(*func)();
};


dy_test dy_tests[] = {
	{"dy_ustack", test_dy_ustack},
	{"mat3::solve", test_dy_mat3_solve},
};
const int dy_test_count = sizeof(dy_tests) / sizeof(dy_test);

int main(int argc, char** args)
{
	if (argc != 2)
	{
		printf("Missing test id argument\n");
		for (int i = 0; i < dy_test_count; i++)
		{
			printf("%d: %s\n", i, dy_tests[i].name);
		}
		return dy_test_count;
	}

	int id = atoi(args[1]);
	if (id > dy_test_count)
	{
		printf("ID too large!\n");
		return dy_test_count;
	}

	dy_test* test = &dy_tests[id];

	printf("Running test %s...\n", test->name);

	int result = test->func();

	if (result)
	{
		printf("Test failed!\n");
	}
	else
	{
		printf("Test passed\n");
	}


	return result;
}