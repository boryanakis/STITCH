test_that("can do pseudoHaploid updates in C++", {

    ## note - with random numbers, below is kind of weird
    ## but just need to confirm equivalency
    srp <- c(0, 7, 9) # 0-based
    n_reads <- length(srp)
    K <- 4
    T <- 10
    set.seed(1)
    pRgivenH1 <- runif(n_reads)
    pRgivenH2 <- runif(n_reads)
    fbsoL <- list(
        list(eMatHap = matrix(runif(K * n_reads), nrow = n_reads)),
        list(eMatHap = matrix(runif(K * n_reads), nrow = n_reads))
    )
    for(iNor in 1:2) {
        fbsoL[[iNor]]$gamma <- matrix(runif(K * T), nrow = T)
        fbsoL[[iNor]]$eMatHap_t <- t(fbsoL[[iNor]]$eMatHap)
        fbsoL[[iNor]]$gamma_t <- t(fbsoL[[iNor]]$gamma)
    }

    ## model 9 - original configuration
    x <- pRgivenH1/(pRgivenH1+pRgivenH2)
    y1 <- (fbsoL[[1]]$eMatHap - (1-x) * pRgivenH2) / x
    y2 <- (fbsoL[[2]]$eMatHap - x * pRgivenH1) / (1 - x)
    pRgivenH1_new <- rowSums(fbsoL[[1]]$gamma[srp + 1,] * y1)
    pRgivenH2_new <- rowSums(fbsoL[[2]]$gamma[srp + 1,] * y2)

    out1 <- pseudoHaploid_update_9(
        pRgivenH1 = pRgivenH1,
        pRgivenH2 = pRgivenH2,
        eMatHap_t1 = fbsoL[[1]]$eMatHap_t,
        eMatHap_t2 = fbsoL[[2]]$eMatHap_t,
        gamma_t1 = fbsoL[[1]]$gamma_t,
        gamma_t2 = fbsoL[[2]]$gamma_t,
        K = K,
        srp = srp
    )

    out2 <- pseudoHaploid_update_model_9(
        pRgivenH1 = pRgivenH1,
        pRgivenH2 = pRgivenH2,
        eMatHap_t1 = fbsoL[[1]]$eMatHap_t,
        eMatHap_t2 = fbsoL[[2]]$eMatHap_t,
        gamma_t1 = fbsoL[[1]]$gamma_t,
        gamma_t2 = fbsoL[[2]]$gamma_t,
        K = K,
        srp = srp
    )

    expect_equal(sum(abs(out1$pRgivenH1 - pRgivenH1_new)), 0)
    expect_equal(sum(abs(out1$pRgivenH2 - pRgivenH2_new)), 0)
    expect_equal(sum(abs(out2$pRgivenH1 - pRgivenH1_new)), 0)
    expect_equal(sum(abs(out2$pRgivenH2 - pRgivenH2_new)), 0)


})


test_that("forwardBackwardDiploid and forwardBackwardHaploid work", {

    n_snps <- 10 ## set to 10000 to check times better
    K <- 20

    phasemaster <- matrix(
        c(rep(0, n_snps), rep(1, n_snps)),
        ncol = K
    )
    data_package <- make_acceptance_test_data_package(
        n_samples = 1,
        n_snps = n_snps,
        n_reads = n_snps * 2,
        seed = 2,
        chr = 10,
        K = K,
        phasemaster = phasemaster,
        reads_span_n_snps = 3,
        n_cores = 1
    )
    ##tmpdir = "/data/smew1/rdavies/stitch_development/STITCH_v1.2.7_development/cppdir/"
    ##save(data_package, file = "/data/smew1/rdavies/stitch_development/STITCH_v1.2.7_development/cppdir/test.RData")
    ##load("/data/smew1/rdavies/stitch_development/STITCH_v1.2.7_development/cppdir/test.RData")

    regionName <- "region-name"
    loadBamAndConvert(
        iBam = 1,
        L = data_package$L,
        pos = data_package$pos,
        nSNPs = data_package$nSNPs,
        bam_files = data_package$bam_files,
        N = 1,
        sampleNames = data_package$sample_names,
        inputdir = tempdir(),
        regionName = regionName,
        tempdir = tempdir(),
        chr = data_package$chr,
        chrStart = 1,
        chrEnd = max(data_package$pos[, 2]) + 100
    )

    load(file_sampleReads(tempdir(), 1, regionName))
    eHaps <- array(runif(n_snps * K), c(n_snps, K))
    sigma <- runif(n_snps - 1)
    alphaMat <- array(runif((n_snps - 1) * K), c(n_snps - 1, K))
    x <- sigma
    transMatRate <- cbind(x ** 2, x * (1 - x), (1 - x) ** 2)
    pi <- runif(K) / K

    ## run through R function
    out <- run_forward_backwards(
        sampleReads = sampleReads,
        priorCurrent = pi,
        transMatRate_t_D = t(transMatRate),
        alphaMatCurrent_t = t(alphaMat),
        eHapsCurrent_t = t(eHaps),
        method = "diploid"
    )

    ## basic checks
    gammaK_t <- out$fbsoL[[1]][["gammaK_t"]]
    expect_equal(ncol(gammaK_t), n_snps)
    expect_equal(min(gammaK_t) >= 0, TRUE)
    expect_equal(max(gammaK_t) <= 1, TRUE)    

    pRgivenH1L <- runif(length(sampleReads))
    pRgivenH2L <- runif(length(sampleReads))

    for(run_pseudo_haploid in c(TRUE, FALSE)) {
        ## run through R function
        out <- run_forward_backwards(
            sampleReads = sampleReads,
            priorCurrent = pi,
            transMatRate_t_H = t(transMatRate),
            alphaMatCurrent_t = t(alphaMat),
            eHapsCurrent_t = t(eHaps),
            pRgivenH1 = pRgivenH1L,
            pRgivenH2 = pRgivenH2L,
            method = "pseudoHaploid"
        )
        ## basic checks
        expect_equal(ncol(out$fbsoL[[1]]$gamma_t), n_snps)
        expect_equal(min(out$fbsoL[[1]]$gamma_t) >= 0, TRUE)
        expect_equal(max(out$fbsoL[[1]]$gamma_t) <= 1, TRUE)    
    
    }


})


test_that("can sample one path from forwardBackwardDiploid", {

    set.seed(10)

    speed_test <- FALSE
    if (speed_test) {
        n_snps <- 25000 ## can set higher
        tmpdir <- "./"
        dir.create(tmpdir, showWarnings = FALSE)
        suppressOutput <- as.integer(0)
        n_reads <- round(n_snps / 10)  ## set to vary coverage
        gridWindowSize <- NA  ## 10 SNPs / grid
    } else {
        gridWindowSize <- NA        
        n_snps <- 10
        n_reads <- n_snps * 2
        tmpdir <- tempdir()
        suppressOutput <- as.integer(1)
    }

    K <- 20    
    phasemaster <- matrix(
        c(rep(0, n_snps), rep(1, n_snps)),
        ncol = K
    )
    file <- file.path(tmpdir, "package.RData")    
    if (speed_test & file.exists(file)) {
        print("loading file!")
        load(file)
    } else {
        data_package <- make_acceptance_test_data_package(
            n_samples = 1,
            n_snps = n_snps,
            n_reads = n_reads,
            seed = 2,
            chr = 10,
            K = K,
            phasemaster = phasemaster,
            reads_span_n_snps = 2,
            n_cores = 16,
            tmpdir = tmpdir
        )
        regionName <- "region-name"
        loadBamAndConvert(
            iBam = 1,
            L = data_package$L,
            pos = data_package$pos,
            nSNPs = data_package$nSNPs,
            bam_files = data_package$bam_files,
            N = 1,
            sampleNames = data_package$sample_names,
            inputdir = tempdir(),
            regionName = regionName,
            tempdir = tempdir(),
            chr = data_package$chr,
            chrStart = 1,
            chrEnd = max(data_package$pos[, 2]) + 100
        )
        load(file_sampleReads(tempdir(), 1, regionName))
        if (speed_test) {
            save(sampleReads, data_package, file = file)
        }
    }

    set.seed(40)
    L <- 1:n_snps
    out <- assign_positions_to_grid(
        L = L,
        gridWindowSize = gridWindowSize
    )
    grid <- out$grid
    grid_distances <- out$grid_distances
    L_grid <- out$L_grid
    nGrids <- out$nGrids
    
    eHaps <- array(runif(n_snps * K), c(n_snps, K))
    sigma <- rep(0.999, nGrids - 1)
    alphaMat <- array(1 / K / K, c(nGrids - 1, K))
    transMatRate_t_D <- get_transMatRate("diploid", sigma)
    transMatRate_t_H <- get_transMatRate("diploid-inbred", sigma)
    pi <- runif(K) / K
    eHaps[, 1] <- 0.01
    eHaps[, 2] <- 0.99
    if (!is.na(gridWindowSize)) {
        sampleReads <- snap_sampleReads_to_grid(
            sampleReads = sampleReads,
            grid = grid
        )
    }
    nSNPs <- n_snps
    priorSum <- array(0, K)
    jUpdate_t <- array(0, c(K, nGrids - 1))
    gammaUpdate_t <- array(0, c(K, nSNPs, 2))
    hapSum_t <- array(0, c(K, nGrids))
    alphaHat_t <- array(0, c(K * K, nGrids))
    betaHat_t <- array(0, c(K * K, nGrids))    
    
    ##out2 <- forwardBackwardDiploid_old(
    ##    sampleReads = sampleReads,
    ##    nReads = as.integer(length(sampleReads)),
    ##    pi = pi,
    ##    transMatRate = t(transMatRate),
    ##    alphaMat = t(alphaMat),
    ##    eHaps = t(eHaps),
    ##    maxDifferenceBetweenReads = as.double(1000),
    ##    maxEmissionMatrixDifference = as.double(1000),
    ##    Jmax = as.integer(10),
    ##    suppressOutput = suppressOutput,
    ##    return_a_sampled_path = TRUE,
    ##    blocks_for_output = array(NA, c(1, 1)),
    ##    whatToReturn = as.integer(0)
    ## )
    set.seed(50)
    out <- forwardBackwardDiploid(
        sampleReads = sampleReads,
        nReads = as.integer(length(sampleReads)),
        pi = pi,
        transMatRate = transMatRate_t_D,
        alphaMat = t(alphaMat),
        eHaps = t(eHaps),
        maxDifferenceBetweenReads = as.double(1000),
        maxEmissionMatrixDifference = as.double(1000),
        Jmax = as.integer(10),
        suppressOutput = suppressOutput,
        return_a_sampled_path = TRUE,
        blocks_for_output = array(NA, c(1, 1)),
        return_gamma = TRUE,
        return_genProbs = TRUE, ## time this as well
        grid = grid,
        snp_start_1_based = 1,
        snp_end_1_based = n_snps,
        return_extra = TRUE,
        update_in_place = TRUE,
        gammaUpdate_t = gammaUpdate_t,
        jUpdate_t = jUpdate_t,
        hapSum_t = hapSum_t,
        priorSum = priorSum,
        pass_in_alphaBeta = TRUE,        
        alphaHat_t = alphaHat_t,
        betaHat_t = betaHat_t,
    )

    ##expect_equal(out1$alphaHat_t, out2$alphaHat_t)
    ##expect_equal(out1$betaHat_t, out2$betaHat_t)
    ##expect_equal(out1$gammaK_t, out2$gammaK_t)
    ##expect_equal(out1$jUpdate_t, out2$jUpdate_t)
    ##expect_equal(out1$gammaUpdate_t, out2$gammaUpdate_t)
    ##print(mean(out1$jUpdate_t - out2$jUpdate_t))
    
    ## basically, these should be the same
    ## given the seed and the ridiculous good fit
    marginally_most_likely_path <- apply(out$gamma_t, 2, which.max) ## 1-based

    sampled_path <- out$sampled_path_diploid_t[3, ]
    ## 0-based,
    ## should be 1, 0 -> 1
    ## or 0, 1 -> 20
    expect_equal(sum(sampled_path != 1 & sampled_path != 20), 0)

    if (speed_test) {
        print("test haploid")
    }
    alphaHat_t <- array(0, c(K, nGrids))
    betaHat_t <- array(0, c(K, nGrids))
    priorSum <- array(0, K)
    jUpdate_t <- array(0, c(K, nGrids - 1))
    gammaUpdate_t <- array(0, c(K, nSNPs, 2))
    hapSum_t <- array(0, c(K, nGrids))
    out1 <- forwardBackwardHaploid(
        sampleReads = sampleReads,
        nReads = as.integer(length(sampleReads)),
        pi = pi,
        transMatRate = transMatRate_t_H,
        alphaMat = t(alphaMat),
        eHaps = t(eHaps),
        maxDifferenceBetweenReads = as.double(1000),
        maxEmissionMatrixDifference = as.double(1000),
        Jmax = as.integer(10),
        suppressOutput = suppressOutput,
        blocks_for_output = array(NA, c(1, 1)),
        return_extra = FALSE,
        update_in_place = TRUE,
        gammaUpdate_t = gammaUpdate_t,
        jUpdate_t = jUpdate_t,
        hapSum_t = hapSum_t,
        priorSum = priorSum,
        pass_in_alphaBeta = TRUE,        
        alphaHat_t = alphaHat_t,
        betaHat_t = betaHat_t,
        model = 1000,
        run_pseudo_haploid = FALSE,
        pRgivenH1 = array(0, 1),
        pRgivenH2 = array(0, 1)
    )
    ## print("OLD")
    ## priorSum2 <- array(0, K)
    ## jUpdate_t2 <- array(0, c(K, nGrids - 1))
    ## gammaUpdate_t2 <- array(0, c(K, nSNPs, 2))
    ## hapSum_t2 <- array(0, c(K, nGrids))
    ## out2 <- forwardBackwardHaploid_TEMP(
    ##     sampleReads = sampleReads,
    ##     nReads = as.integer(length(sampleReads)),
    ##     pi = pi,
    ##     transMatRate = transMatRate_t_H,
    ##     alphaMat = t(alphaMat),
    ##     eHaps = t(eHaps),
    ##     maxDifferenceBetweenReads = as.double(1000),
    ##     maxEmissionMatrixDifference = as.double(1000),
    ##     Jmax = as.integer(10),
    ##     suppressOutput = 0,
    ##     blocks_for_output = array(NA, c(1, 1)),
    ##     return_extra = FALSE,
    ##     update_in_place = TRUE,
    ##     gammaUpdate_t = gammaUpdate_t2,
    ##     jUpdate_t = jUpdate_t2,
    ##     hapSum_t = hapSum_t2,
    ##     priorSum = priorSum2,
    ##     pass_in_alphaBeta = TRUE,        
    ##     alphaHat_t = alphaHat_t,
    ##     betaHat_t = betaHat_t,
    ##     model = 1000,
    ##     run_pseudo_haploid = FALSE,
    ##     pRgivenH1 = array(0, 1),
    ##     pRgivenH2 = array(0, 1)
    ## )
    ## expect_equal(out1, out2)
    ## expect_equal(priorSum, priorSum2)
    ## expect_equal(jUpdate_t2, jUpdate_t)
    ## expect_equal(gammaUpdate_t2, gammaUpdate_t)
    ## expect_equal(hapSum_t, hapSum_t2)
    
    

})


test_that("can calculate eMatHapSNP and sample a haploid path", {

    n_snps <- 10 ## set to 10000 to check times better
    K <- 4
    phasemaster <- matrix(
        c(rep(0, n_snps), rep(1, n_snps)),
        ncol = K
    )
    data_package <- make_acceptance_test_data_package(
        n_samples = 1,
        n_snps = n_snps,
        n_reads = n_snps * 2,
        seed = 2,
        chr = 10,
        K = K,
        phasemaster = phasemaster,
        reads_span_n_snps = 3,
        n_cores = 1
    )

    regionName <- "region-name"
    loadBamAndConvert(
        iBam = 1,
        L = data_package$L,
        pos = data_package$pos,
        nSNPs = data_package$nSNPs,
        bam_files = data_package$bam_files,
        N = 1,
        sampleNames = data_package$sample_names,
        inputdir = tempdir(),
        regionName = regionName,
        tempdir = tempdir(),
        chr = data_package$chr,
        chrStart = 1,
        chrEnd = max(data_package$pos[, 2]) + 100
    )

    load(file_sampleReads(tempdir(), 1, regionName))
    eHaps <- array(runif(n_snps * K), c(n_snps, K))

    eMatHap_t <- rcpp_make_eMatHap_t(
        sampleReads = sampleReads,
        nReads = length(sampleReads),
        eHaps_t = t(eHaps),
        maxDifferenceBetweenReads = 1000,
	Jmax = 10,
	eMatHapOri_t = array(0, c(1, 1)),
	pRgivenH1 = array(NA, c(1, 1)),
	pRgivenH2 = array(NA, c(1, 1)),
	run_pseudo_haploid = FALSE
    )

    sigma <- runif(n_snps - 1)
    alphaMat <- array(runif((n_snps - 1) * K), c(n_snps - 1, K))
    x <- sigma
    transMatRate <- cbind(x, 1 - x)
    pi <- runif(K) / K

    read_labels <- as.integer(runif(length(sampleReads)) < 0.5)

    path <- rcpp_sample_path(
        read_labels = read_labels,
        eMatHap_t = eMatHap_t,
        sampleReads = sampleReads,
        nReads = length(sampleReads),
        eHaps_t = t(eHaps),
        maxDifferenceBetweenReads = 1000,
        Jmax = 10,
        pi = pi,
        transMatRate_t_H = t(transMatRate),
        alphaMat_t = t(alphaMat)
    )

    expect_equal(nrow(path), n_snps)
    

})
