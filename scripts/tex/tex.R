library("reshape")
library("xtable")
library("ggplot2")

PATH="/Users/esiragusa/Datasets/ibench"
FILENAME_CONSTRUCT=paste(paste(PATH, "construct", sep='/'), "tsv", sep='.')
FILENAME_VISIT=paste(paste(PATH, "visit", sep='/'), "tsv", sep='.')
FILENAME_QUERY=paste(paste(PATH, "query", sep='/'), "tsv", sep='.')

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


### PLOT VISIT ###

if ((R = load_file(FILENAME_VISIT))$ok)
{
  TABLE_VISIT = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_VISIT))

table_visit = subset(TABLE_VISIT, alphabet=='dna', select=c(index, depth, time))
#fillup = data.frame(depth=c(1:10), rate=log(c(1:10)))

ggplot() +
  geom_line(data=table_visit, aes(x=depth, y=time, group=index, shape=index, color=index)) +
  geom_point(data=table_visit, aes(x=depth, y=time, group=index, shape=index, color=index), size=3) +
  xlab("visit depth") +
  ylab("visit time") +
  scale_y_log10() +
  theme_bw(base_size=12, base_family="Helvetica")


### PLOT QUERY ###

if ((R = load_file(FILENAME_QUERY))$ok)
{
  TABLE_QUERY = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_QUERY))

table_query = subset(TABLE_QUERY, alphabet=='dna' & errors==0, select=c(index, plength, time))
#fillup = data.frame(depth=c(1:10), rate=log(c(1:10)))

ggplot() +
  geom_line(data=table_query, aes(x=plength, y=time, group=index, shape=index, color=index)) +
  geom_point(data=table_query, aes(x=plength, y=time, group=index, shape=index, color=index), size=3) +
  xlab("pattern length") +
  ylab("average time") +
  theme_bw(base_size=12, base_family="Helvetica")
#+ geom_line(data=fillup, aes(x=depth, y=rate), linetype='dashed')

