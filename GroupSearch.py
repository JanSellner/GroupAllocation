import numpy as np

from GroupEvaluation import GroupEvaluation
from MeasureTime import MeasureTime


class GroupSearch:
    def __init__(self, n_groups: int, n_students: int, foreigners=None):
        self.groups = np.arange(1, n_groups + 1)
        self.n_students = n_students

        self.gval = GroupEvaluation(self.groups, self.n_students, foreigners)

    def find_best_allocation(self):
        errors = []
        last_improvements = []
        best_error = np.inf
        best_combs = None

        with MeasureTime():
            for _ in range(20):
                error, last_improvement, combs = self._start_random_walk()
                errors.append(error)
                last_improvements.append(last_improvement)

                if sum(error) < best_error:
                    best_error = sum(error)
                    best_combs = combs

        return self.gval.add_last_comb(best_combs), best_error

    def _start_random_walk(self):
        # Random combination for all students (columns) and all days (rows)
        combs = np.stack([np.random.choice(self.groups, size=2, replace=False) for _ in range(self.n_students)]).transpose()

        error = self.gval.error_total(combs)
        last_improvement = -1

        for i in range(200):
            idx_student = np.random.randint(0, self.n_students)
            new_slots = np.random.choice(self.groups, 2, replace=False)

            # Temporarily assign the student to new groups (for all days)
            combs_copy = combs.copy()
            combs_copy[:, idx_student] = new_slots

            error_new = self.gval.error_total(combs_copy)
            if sum(error_new) < sum(error):
                error = error_new
                combs = combs_copy
                last_improvement = i

        return error, last_improvement, combs
