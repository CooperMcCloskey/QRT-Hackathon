# Probe strategy

## What is established

The judge accepted scalar outputs 0 and 1 on all available cases. We have
captured numeric feedback only at t=0 for those two experiments. The payoff
formula, noise distribution, meaning of N, and legal domain are still unknown.
There is no universally optimal finite-query method for an unrestricted
unknown function. The practical objective is to identify useful structure
quickly, test it, and then optimise it.

## Collect a whole response curve per run

Every Run Code already evaluates the available input cases. Save all raw
feedback, not only a screenshot of one case. Preserve the current batch
before replacing the editor. Use raw_noisy_score for inference: clipping the
normalized score loses the magnitude of negative observations and values
above the target. The target is a diagnostic reference, not an extra query.

Each experiment must retain t, output array, N, raw payoff, and target.
Do not merge distinct inputs merely because displayed t is rounded to two
decimals. Exact input and candidate values are printed by diagnostic probes.

## Paired, controlled experiments

probe_v4 chooses a signed amplitude x(t) for each input. probe_v5 uses its
negative at exactly the same t. Different t values must not be treated as
replicates of one objective. The varying amplitudes explore scale; they are
not a statistically optimal design established by the current evidence.

Start by testing the hypothesis

    E[Y(t, x)] = c(t) + a(t)*x - b(t)*x*x, with b(t) > 0.

For equal positive amplitude h and averaged observations y_plus, y_minus, y_zero:

    a_hat = (y_plus - y_minus) / (2*h)
    b_hat = (2*y_zero - y_plus - y_minus) / (2*h*h)
    x_stationary = a_hat / (2*b_hat)

These formulas identify an exact quadratic from noiseless data. Noisy data
produce estimates, not a guarantee of optimality. Three values always fit a
quadratic, so use an additional amplitude to test its prediction. Reject a
concave-quadratic model if b is not convincingly positive or residuals are
larger/structured beyond measured noise. Do not extrapolate to an untested
domain on a nearly-zero curvature estimate.

With independent additive noise of common variance sigma^2 and m repetitions
per sign, Var(a_hat) = sigma^2 / (2*m*h*h). Larger h helps under this assumption,
but domain limits, nonquadratic terms, and output-dependent noise can reverse
that tradeoff. Compare repeated observations and two amplitudes before tuning h.
The one zero observation does not establish a zero intercept or zero noise.

## Determine array semantics early

Compare [x], [x, 0], [0, x], [x, x], and [x, -x] at matched t, recording N and
target changes as well as payoff. Test [0, 0] as the two-coordinate reference.
Do not assume a returned array is a batch of independent oracle queries.
For N=2, the contrast f(x,x)-f(x,0)-f(0,x)+f(0,0) tests interaction at that scale;
it is zero for an additive objective, subject to sampling noise.
Hold N fixed when estimating derivatives; changing N may change the objective.

## Reduce noise through a model only if the data support it

Plot/inspect the inferred response against exact t. Fit a small smooth or
periodic model only if it predicts held-out observations. Use regularisation
and validation to avoid fitting noise. Periodicity is a lead from the indexed
Imperial write-up, not a property established for this judge.
Never embed a table of observed test answers: the contest protocol prohibits
precomputed answers without organiser permission. A general model must be
validated on additional probes and inputs.

If the objective is truly multidimensional and smooth, use paired random
perturbations (SPSA) to estimate a gradient with two evaluations per iteration.
This is a local optimisation method, not a guarantee of a global optimum.
If the low-dimensional objective is nonconcave, explore multiple regions
before refining promising ones.

## Final selection

Compare candidates using fresh repeated observations. A high individual noisy
reading is not a reliable selection criterion. Allocate additional evaluations
where uncertainty can change which candidate wins. Preserve a valid fallback,
and track the official final noiseless result separately from probe feedback.

References:
- NIST response surface designs: https://www.itl.nist.gov/div898/handbook/pri/section3/pri336.htm
- Spall's original SPSA paper: https://www.jhuapl.edu/spsa/pdf-spsa/spall_tac92.pdf
