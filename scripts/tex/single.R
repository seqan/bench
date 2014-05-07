library("reshape")
library("xtable")
library("ggplot2")

PATH="/Users/esiragusa/Code/seqan/core/apps/ibench/resources"
FILENAME=paste(paste(PATH, "single", sep='/'), "tsv", sep='.')

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

if ((R = load_file(FILENAME))$ok)
{
  TSV = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME))


### PLOT TIMES ###

dna_0 = subset(TSV, alphabet=dna, errors=0, select=c(index, plength, time))
fillup = data.frame(depth=c(1:10), rate=log(c(1:10)))

ggplot() +
  geom_line(data=dna_0, aes(x=plength, y=time, group=index, shape=index, color=index)) +
  geom_point(data=dna_0, aes(x=plength, y=time, group=index, shape=index, color=index), size=3) +
  xlab("pattern length") +
  ylab("average time") +
  theme_bw(base_size=12, base_family="Helvetica") +
  geom_line(data=fillup, aes(x=depth, y=rate), linetype='dashed')
