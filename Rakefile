BUILD_DIR = File.join(__dir__, 'build', 'debug')
PWD = File.expand_path(__dir__)

directory BUILD_DIR do
  sh "cmake -S #{PWD} -B #{BUILD_DIR} -G Ninja -DCMAKE_BUILD_TYPE=Debug"
end

desc "Compile the project"
task compile: BUILD_DIR do
  sh "cmake --build #{BUILD_DIR} --parallel"
end

desc "Run the project"
task run: :compile do
  sh File.join(BUILD_DIR, 'Raptor')
end

desc "Watch for changes and recompile"
task :watch do
  require 'listen'
  listener = Listen.to('src', 'include') do |_modified, _added, _removed|
    Rake::Task[:compile].execute
  end
  puts "Watching for changes in 'src' and 'include' directories..."
  listener.start
  sleep
end

task default: :compile
