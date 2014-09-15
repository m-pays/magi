module paras_m
    ! integer
    integer, parameter :: i4b = 4
    integer, parameter :: i2b = 2
    integer, parameter :: i1b = 1
    ! single- and double-precision real
    integer, parameter :: sp = 4
    integer, parameter :: dp = 8
    ! single- and double-precision complex
    integer, parameter :: spc = 4
    integer, parameter :: dpc = 8
    ! logical type
    integer, parameter :: lgc = 4
    real(dp), parameter :: pi        = 3.141592653589793238462643383279502884197_dp
    real(dp), parameter :: AZero     = 1.d-200
    real(dp), parameter :: AInfinity = 1.d200
end module paras_m

module spectral_weight_m
    implicit none
contains
    function exp_n(xt) result(exp_rst)
	use paras_m 
	implicit none
	real(dp), intent(in)    :: xt
	real(dp)                :: exp_rst
	real(dp) :: p1, p3, p4, p6
	p1 = -700._dp
	p3 = -0.8d-8
	p4 = 0.8d-8
	p6 = 700._dp
	if(xt < p1+epsilon(p1)) then
	    exp_rst = 0._dp
	else if(xt > p6 - epsilon(p6)) then
	    exp_rst = AInfinity
	else if(xt > p3 .and. xt < p4) then
	    exp_rst = 1._dp + xt
	else
	    exp_rst = exp(xt)
	end if
    end function exp_n

    subroutine msg_error(errstr, rtnstr)
        implicit none
        character(len=*), intent(in) :: errstr
        character(len=*), intent(in) :: rtnstr
        print *, trim(errstr) // repeat(" ", 1) // "--> [" // trim(rtnstr) // "]."
        write(*, '(A)', advance='no') " (!!) Program terminated; Press return to exit. "
        read(*,*);  stop
    end subroutine msg_error

    function assert_eq(na, string)
        use paras_m               ! numerical kind values
        implicit none
        integer(i4b), dimension(:), intent(in) :: na
        character(len=*), intent(in) :: string
        integer(i4b) :: assert_eq
        logical :: errflag
        integer(i4b) :: i
        errflag = .false.
        assert_eq = 0
        do i = 2, size(na)
            if(na(i) /= na(1)) errflag = .true.
        end do
        if (.not. errflag) then
            assert_eq = na(1)
        else
            call msg_error('An assertion failed with this tag: ', string)
        end if
    end function assert_eq

    function arth(first, increment, n)
        use paras_m
        implicit none
        real(dp), intent(in)        :: first, increment
        integer(i4b), intent(in)    :: n
        real(dp), dimension(n)      :: arth
        integer(i4b) :: k
        if (n > 0) arth(1)=first
        do k=2,n
            arth(k)=arth(k-1)+increment
        end do
    end function arth

    subroutine gauleg(x1, x2, x, w)
        use paras_m
        implicit none
        real(dp), intent(in)                :: x1, x2
        real(dp), dimension(:), intent(out) :: x, w
        integer(i4b)    :: its,j,m,n, maxIT
        real(dp)        :: xl,xm, eps2
        real(dp), dimension((size(x)+1)/2)  :: p1,p2,p3,pp,z,z1
        logical(lgc), dimension((size(x)+1)/2)  :: unfinished
        eps2=3.0d-14
        maxIT=10
        n=assert_eq((/size(x),size(w)/), 'gauleg')
        m=(n+1)/2                               ! The roots are symmetric in the interval,
        xm=0.5_dp*(x2+x1)                       ! so we only have to find half of them
        xl=0.5_dp*(x2-x1)
        z=cos(pi*(arth(1._dp,1._dp,m)-0.25_dp)/(n+0.5_dp))    ! Initial approximations to the roots.
        unfinished=.true.
        do its=1,maxIT                          ! Newton¡¯s method carried out simultaneously on the roots
            where(unfinished)
                p1=1.0
                p2=0.0
            end where
            do j=1,n                            ! Loop up the recurrence relation to get the Legendre
                where (unfinished)              ! polynomials evaluated at z
                    p3=p2
                    p2=p1
                    p1=((2.0_dp*j-1.0_dp)*z*p2-(j-1.0_dp)*p3)/j
                end where
            end do
            ! p1 now contains the desired Legendre polynomials. We next compute pp, the derivatives,
            ! by a standard relation involving also p2, the polynomials of one lower order.
            where (unfinished)
                pp=n*(z*p1-p2)/(z*z-1.0_dp)
                z1=z
                z=z1-p1/pp                      ! Newton method.
                unfinished=(abs(z-z1) > eps2)
            end where
            if (.not. any(unfinished)) exit
        end do
        if (its == maxIT+1) call msg_error("Error: too many iterations in gauleg", "gauleg")
        x(1:m)=xm-xl*z                          ! Scale the root to the desired interval,
        x(n:n-m+1:-1)=xm+xl*z                   ! and put in its symmetric counterpart.
        w(1:m)=2.0_dp*xl/((1.0_dp-z**2)*pp**2)  ! Compute the weight
        w(n:n-m+1:-1)=w(1:m)                    ! and its symmetric counterpart.
    end subroutine gauleg

    function GaussianQuad_N(func, a2, b2, divs2, NptGQ) result(GaussQ_rst)
        use paras_m
        implicit none
        real(dp), intent(in)        :: a2, b2
        integer(i4b), intent(in)    :: divs2, NptGQ
        real(dp)                    :: GaussQ_rst
        interface
            function func(x) result(func_rst)
                use paras_m
                real(dp), intent(in) :: x
                real(dp) :: func_rst
            end function func
        end interface
        integer(i4b)                :: i, j, divs
        real(dp)                    :: dh, a0, b0
        real(dp), dimension(NptGQ)  :: fxpts, weights
        GaussQ_rst = 0._dp
        if(abs(a2-b2) < AZero) return
        divs = divs2
        if (divs2 < 1) then
            print *, "Warning: incorrect subinterval numbers for quadrature."
            divs = 1_i4b
        end if
        dh = (b2-a2) / real(divs, dp)
        do i = 1, divs
            a0 = a2 + real(i-1, dp)*dh
            b0 = a0 + dh
            call gauleg(a0, b0, fxpts, weights)  ! generate the abscissas and weights for Gauss-Legendre N-point quadrature formula
!            call gauleg(-1._dp, 1._dp, fxpts, weights)  ! generate the abscissas and weights for Gauss-Legendre N-point quadrature formula
            do j = 1, NptGQ
                fxpts(j) = func(fxpts(j))
!                fxpts(j) = func( (b0-a0) * fxpts(j) / 2._dp + (b0+a0) / 2._dp ) * (b0-a0) /2._dp
            end do
            GaussQ_rst = GaussQ_rst + sum(weights * fxpts)
        end do
    end function GaussianQuad_N

    function sw_integ(wvnmb) result(rst)
        use paras_m
        implicit none
        real(dp), intent(in) :: wvnmb
        real(dp)             :: rst
	rst = ( (5.55243*( exp_n(-0.3*wvnmb/15.762) - exp_n(-0.6*wvnmb/15.762) ) )*wvnmb )**0.5_dp / 8.61553 * (sin(wvnmb/65._dp))**2._dp;
    end function sw_integ

    function sw(nounce, divs) result(sw_rst)
        use paras_m
        implicit none
!        real(dp), intent(in)     :: wmin, wmax
        integer(i4b), intent(in) :: nounce, divs
        real(dp)                 :: wmax, sw_rst
!	sw_rst = sw_integ(wmax)
	wmax = (abs(real(nounce)))**(0.5_dp)/80._dp + 100._dp
        sw_rst = GaussianQuad_N(sw_integ, 0._dp, wmax, divs, 3)
        sw_rst = abs(sw_rst) + 1._dp + epsilon(1._dp)
    end function sw
end module spectral_weight_m
