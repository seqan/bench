library("reshape")
library("xtable")
library("ggplot2")

options(scipen=999)

checkNA <- function(y)
{
    any(is.na(y))
}

load_file <- function(filename)
{
    if (file.exists(filename))
    {
        # try the main resource file first
        print(paste("read",filename))
        tsvFile = try(read.delim(filename, header=TRUE))
        if (inherits(tsvFile,"try-error"))
        {
            print(paste("EMPTY FILE:", filename))
            return(list(ok=FALSE))
        }
        if (nrow(tsvFile) < 1)
        {
            print(paste("BAD FORMAT:", filename))
            return(list(ok=FALSE))
        }
        if (any(apply(tsvFile, 1, checkNA)))
        {
            print(paste("BAD FORMAT:", filename))
            return(list(ok=FALSE))
        }
        return(list(ok=TRUE,tsv=tsvFile))
    } else
        return(list(ok=FALSE))
}

ALGO = c("single","dfs")
DISTANCE = c("hamming","edit")
ORGA = c("ecoli","dmel","celegans","hg18")
ORGA_NAME = c("E.\\,coli","D.\\,melanogaster","C.\\,elegans", "H.\\,sapiens")
SEEDLEN = c(15,30)
SEEDERR = seq(0,2)
SEEDCNT = c(10000,100000,1000000,10000000)

x=c()

for (a in seq(length(ALGO)))
{
    algo=ALGO[a]
    for (d in seq(length(DISTANCE)))
    {
        distance=DISTANCE[d]
        for (o in seq(length(ORGA)))
        {
            orga=ORGA[o]
            for (l in seq(length(SEEDLEN)))
            {
                seedlen=SEEDLEN[l]
                for (e in seq(length(SEEDERR)))
                {
                    seederr=SEEDERR[e]
                    for (c in seq(length(SEEDCNT)))
                    {
                        seedcnt=SEEDCNT[c]
#                        PATH="/data/scratch/NO_BACKUP/esiragusa/backtracking/resources/"
                        PATH="/Users/esiragusa/Volumes/Allegro/backtracking/resources/"
                        FILENAME=paste(paste(PATH, algo, sep=''), distance, orga, seedlen, seederr, seedcnt, "tsv", sep='.')
                        
                        if ((R = load_file(FILENAME))$ok)
                        {
                            line = R$tsv;
                            if (!is.na(line$res_mem_peak[1]))
                                x <- rbind(x, line)
                        }
                        else
                            print(paste("NOT FOUND:", FILENAME))
                    }
                }
            }
        }
    }
}


### PRINT TIMES ###

y=cast(x, orga+seedlen+seedcnt~distance+seederr+algo, value='totaltime')
colnames(y)=gsub('_', '-', colnames(y), fixed = TRUE)
colnames(y)=gsub('hamming', 'H', colnames(y), fixed = TRUE)

T=xtable(y)

print(T,
      file='tables/times.tex',
      floating=FALSE,
      sanitize.colnames.function=identity,
      include.rownames=FALSE,
      hline.after=NULL,
      NA.string="--",
      add.to.row=list(pos=list(-1,0, nrow(T)),
                      command=c(
                        paste(
                          '\\toprule',
                          paste('\\multicolumn{3}{c}{dataset}', '\\multicolumn{4}{c}{algorithm}\\\\', sep=' &'),
                          '\\cmidrule{1-3} \\cmidrule{4-7}',
                          sep='\n'),
                        '\\midrule ',
                        '\\bottomrule ')))


### PRINT HITS ###

z=cast(x, orga+seedlen+seedcnt~distance+seederr+algo, value='hits')
colnames(z)=gsub('_', '-', colnames(z), fixed = TRUE)
colnames(z)=gsub('hamming', 'H', colnames(z), fixed = TRUE)

T=xtable(z,digits=0)

print(T,
      file='tables/hits.tex',
      floating=FALSE,
      sanitize.colnames.function=identity,
      include.rownames=FALSE,
      hline.after=NULL,
      NA.string="--",
      add.to.row=list(pos=list(-1,0, nrow(T)),
                      command=c(
                        paste(
                          '\\toprule',
                          paste('\\multicolumn{3}{c}{dataset}', '\\multicolumn{4}{c}{algorithm}\\\\', sep=' &'),
                          '\\cmidrule{1-3} \\cmidrule{4-7}',
                          sep='\n'),
                        '\\midrule ',
                        '\\bottomrule ')))


### PRINT SPEEDUP ###

# w=cast(x, orga+seedlen+seedcnt~distance+seederr, value='totaltime', FUNC=)
# colnames(w)=gsub('_', '-', colnames(w), fixed = TRUE)
# colnames(w)=gsub('hamming', 'H', colnames(w), fixed = TRUE)
# 
# T=xtable(w)
# 
# print(T,
#       file='tables/speedup.tex',
#       floating=FALSE,
#       sanitize.colnames.function=identity,
#       include.rownames=FALSE,
#       hline.after=NULL,
#       NA.string="--",
#       add.to.row=list(pos=list(-1,0, nrow(T)),
#                       command=c(
#                         paste(
#                           '\\toprule',
#                           paste('\\multicolumn{3}{c}{dataset}', '\\multicolumn{4}{c}{algorithm}\\\\', sep=' &'),
#                           '\\cmidrule{1-3} \\cmidrule{4-7}',
#                           sep='\n'),
#                         '\\midrule ',
#                         '\\bottomrule ')))


### PLOT TIMES ###

for (o in seq(length(ORGA)))
{
  orga_=ORGA[o]
  for (l in seq(length(SEEDLEN)))
  {
    seedlen_=SEEDLEN[l]
    
    p = subset(x, orga==orga_ & seedlen==seedlen_, select = c(seedcnt, distance, seederr, algo, totaltime))
    p = within(p, algorithm <- paste(p$distance, p$seederr, p$algo, sep="-"))
    p = subset(p, select = c(seedcnt, algorithm, totaltime))
    
    ggplot(p, aes(seedcnt,totaltime)) + geom_line(aes(colour = algorithm)) + scale_x_log10(breaks=SEEDCNT) + scale_y_log10(breaks=c(0.1,1,10,100,1000,10000)) #ylim(0,3600)
    OUTFILE= paste('plots/times_', paste(paste(orga_, seedlen_, sep='_') , 'pdf', sep='.'), sep='')
    ggsave(OUTFILE,width=10,height=10)
  }
}


### PLOT HITS ###

for (o in seq(length(ORGA)))
{
  orga_=ORGA[o]
  for (l in seq(length(SEEDLEN)))
  {
    seedlen_=SEEDLEN[l]
    
    p = subset(x, orga==orga_ & seedlen==seedlen_, algo='single', select = c(seedcnt, distance, seederr, hits))
    p = within(p, parameter <- paste(p$distance, p$seederr, sep="-"))
    p = subset(p, select = c(seedcnt, parameter, hits))
    
    ggplot(p, aes(seedcnt,hits)) + geom_line(aes(colour = parameter)) + scale_x_log10(breaks=SEEDCNT) + scale_y_log10()
    OUTFILE= paste('plots/hits_', paste(paste(orga_, seedlen_, sep='_') , 'pdf', sep='.'), sep='')
    ggsave(OUTFILE)
  }
}
